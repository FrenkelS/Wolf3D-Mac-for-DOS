package com.sfprod.macwolfwad;

import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import com.sfprod.utils.NumberUtils;

public class MacWolfWadFactory {

	/* Burger resources */

	private static final int rMacPlayPic = 129; /* Mac play logo */
	private static final int rTitlePic = 133; /* Title screen picture */
	private static final int MySoundList = 135; /* List of sound effects to log */
	private static final int MyDarkData = 136; /* 256 byte table to darken walls */
	private static final int MyWallList = 137; /* All wall shapes */
	private static final int MyBJFace = 138; /* BJ's face for automap */
	private static final int rIntermission = 139; /* Intermission background */
	private static final int rInterPics = 141; /* BJ's intermission pictures */
	private static final int rFaceShapes = 142; /* All the permanent game shapes */
	private static final int rFace512 = 143; /* All game sprites */
	private static final int rFace640 = 144;
	private static final int rMapList = 146; /* Map info data */
	private static final int rSongList = 147; /* Music list data */
	private static final int rGetPsychPic = 148;
	private static final int rYummyPic = 149;

	private WadFile wadFile;

	public static void main(String... args) {
		MacWolfWadFactory macWolfWadFactory = new MacWolfWadFactory();
		macWolfWadFactory.createWad(Episode.FIRST_ENCOUNTER);
	}

	static byte[] getBytes(String filename) {
		try {
			return MacWolfWadFactory.class.getResourceAsStream('/' + filename).readAllBytes();
		} catch (IOException e) {
			throw new UncheckedIOException(e);
		}
	}

	void createWad(Episode episode) {
		ResourceFile resourceFile = new ResourceFile(episode);

		Type brgr = resourceFile.getType("BRGR");
		Type mapBrgr = getMapBrgr(episode);
		wadFile = new WadFile(brgr.calculateMaxId() + 1);
		processBurger(brgr, mapBrgr);

		addMusic(episode);

		processCompressedSound(resourceFile.getType("csnd"));
		processUncompressedSound(resourceFile.getType("snd "));
		wadFile.removeLump(MySoundList);
		addPcSpeakerSoundEffects();

		// Take BJ Map from the First Encounter and put it in the Second Encounter
		wadFile.setLump(MyBJFace, new Lump("BJ Map", getBytes("BJ Map")));

		wadFile.removeLump(127); // New Game Pal
		wadFile.removeLump(199); // Pause shape

		wadFile.saveWadFile(episode);
	}

	private Type getMapBrgr(Episode episode) {
		if (episode.getMapInputFilename() == null) {
			return null;
		}

		ResourceFile resourceFile = new ResourceFile(episode.getMapInputFilename());
		return resourceFile.getType("BRGR");
	}

	private void processBurger(Type brgr, Type mapBrgr) {
		for (Resource resource : brgr.resourceList()) {
			Lump lump = new Lump(resource.getName(), resource.getData());
			wadFile.setLump(resource.id(), lump);
		}

		if (mapBrgr != null) {
			for (Resource resource : mapBrgr.resourceList()) {
				Lump lump = new Lump(resource.getName(), resource.getData());
				wadFile.setLump(resource.id(), lump);
			}
		}

		processFaceShapes();
		processBJIntermissionPictures();
		processSprites();
		processWalls();

		swapShortEndianness(rMapList);
		swapShortEndianness(rSongList);

		resize(rMacPlayPic);
		resize(rTitlePic);
		resize(rIntermission);

		decompress(rGetPsychPic);
		Lump getPsych = wadFile.getLump(rGetPsychPic);
		swapShortEndianness(getPsych.data(), 2);

		decompress(rYummyPic);

		makeRaw(rMacPlayPic);
		makeRaw(rTitlePic);
		makeRaw(rIntermission);
		makeRaw(rYummyPic);
	}

	private void processFaceShapes() {
		Lump lumpFaceShapes = wadFile.getLump(rFaceShapes);
		byte[] decompressedFaceShapesData = DLZSS(lumpFaceShapes, 4);

		ByteBuffer bb = ByteBuffer.wrap(decompressedFaceShapesData);
		List<Integer> offsets = new ArrayList<>(47);
		for (int i = 0; i < 47; i++) {
			offsets.add(bb.getInt());
		}

		for (int newlumpnum = 0; newlumpnum < 47; newlumpnum++) {
			int offset = offsets.get(newlumpnum);
			int nextoffset = newlumpnum == 47 - 1 ? decompressedFaceShapesData.length : offsets.get(newlumpnum + 1);
			int length = nextoffset - offset;
			bb.position(offset);
			byte[] data = new byte[length];
			bb.get(data);

			if (12 <= newlumpnum && newlumpnum <= 35) {
				// weapon graphics
				swapShortEndianness(data, 4);
			} else {
				// status bar graphics
				swapShortEndianness(data, 2);
			}

			Lump newlump = new Lump("Face" + newlumpnum, data);
			wadFile.setLump(newlumpnum, newlump);
		}

		Lump lumpFace512 = wadFile.getLump(rFace512);
		byte[] decompressedFace512Data = DLZSS(lumpFace512, 4);

		ByteBuffer bb512 = ByteBuffer.wrap(decompressedFace512Data);
		List<Integer> offsets512 = new ArrayList<>(47 + 10);
		for (int i = 0; i < 47 + 10; i++) {
			offsets512.add(bb512.getInt());
		}

		for (int newlumpnum = 47; newlumpnum < 47 + 10; newlumpnum++) {
			int offset = offsets512.get(newlumpnum);
			bb512.position(offset);
			byte[] data = new byte[2 + 2 + 12 * 22];
			bb512.get(data);
			byte[] resizedData = resize(data, 7, 11);
			Lump newlump = new Lump("Face" + newlumpnum, resizedData);
			wadFile.setLump(newlumpnum, newlump);
		}

		wadFile.removeLump(rFaceShapes);
		wadFile.removeLump(rFace512);
		wadFile.removeLump(rFace640);
	}

	private void processBJIntermissionPictures() {
		Lump lump = wadFile.getLump(rInterPics);
		byte[] decompressedData = DLZSS(lump, 4);

		ByteBuffer bb = ByteBuffer.wrap(decompressedData);

		for (int newlumpnum = 0; newlumpnum < 3; newlumpnum++) {
			int length = 2 + 2 + 142 * 131;
			byte[] data = new byte[length];
			bb.get(12 + newlumpnum * length, data);
			byte[] resizedData = resize(data, 82, 74);
			Lump newlump = new Lump("BJPic" + newlumpnum, resizedData);
			wadFile.setLump(rInterPics + newlumpnum, newlump);
		}
	}

	private void processSprites() {
		for (int lumpnum = 428; lumpnum < wadFile.getLumpCount(); lumpnum++) {
			Lump lump = wadFile.getLump(lumpnum);
			if (!lump.isEmpty()) {
				byte[] decompressedData = DLZSS(lump, 2);
				ByteBuffer bigEndian = ByteBuffer.wrap(decompressedData);
				ByteBuffer littleEndian = ByteBuffer.wrap(decompressedData.clone());
				littleEndian.order(ByteOrder.LITTLE_ENDIAN);
				short width = bigEndian.getShort();
				littleEndian.putShort(width);
				List<Short> columnOffsets = new ArrayList<>();
				for (int i = 0; i < width; i++) {
					short columnOffset = bigEndian.getShort();
					littleEndian.putShort(columnOffset);
					columnOffsets.add(columnOffset);
				}

				for (short columnOffset : columnOffsets) {
					bigEndian.position(columnOffset);
					littleEndian.position(columnOffset);
					short s;
					do {
						s = bigEndian.getShort();
						littleEndian.putShort(s);
					} while (s != (short) -1);
				}

				Lump decompressedLump = new Lump(lump.name(), littleEndian.array());
				wadFile.setLump(lumpnum, decompressedLump);
			}
		}
	}

	private void processWalls() {
		List<Lump> decompressedWallLumps = new ArrayList<>();
		for (int i = 0; i < 64; i++) {
			Lump wallLump = wadFile.getLump(300 + i);
			if (wallLump.isEmpty()) {
				decompressedWallLumps.add(wallLump);
			} else {
				byte[] decompressedData = DLZSS(wallLump, 0);
				decompressedWallLumps.add(new Lump(wallLump.name(), decompressedData));
			}
			wadFile.removeLump(300 + i);
		}

		Lump wallListLump = wadFile.getLump(MyWallList);
		swapShortEndianness(wallListLump.data(), wallListLump.length() / 2);

		ByteBuffer bb = ByteBuffer.wrap(wallListLump.data());
		bb.order(ByteOrder.LITTLE_ENDIAN);
		List<Short> wallResourceNumbers = new ArrayList<>();
		short count = bb.getShort();
		for (int i = 0; i < count; i++) {
			wallResourceNumbers.add(bb.getShort());
		}

		Lump darkDataLump = wadFile.getLump(MyDarkData);
		byte[] darkData = darkDataLump.data();
		wadFile.removeLump(MyDarkData);

		ByteBuffer newbb = ByteBuffer.allocate(2 + 64 * 2);
		newbb.order(ByteOrder.LITTLE_ENDIAN);
		newbb.putShort(count);

		short index = 300;
		for (short wallResourceNumber : wallResourceNumbers) {
			if (wallResourceNumber == 0) {
				newbb.putShort(wallResourceNumber);
			} else {
				Lump wallLump = decompressedWallLumps.get((wallResourceNumber & 0x3fff) - 300);
				Lump newWallLump;
				if ((wallResourceNumber & 0x8000) == 0x8000) {
					byte[] darkendData = new byte[128 * 128];
					for (int j = 0; j < 128 * 128; j++) {
						darkendData[j] = darkData[NumberUtils.toInt(wallLump.data()[j])];
					}
					newWallLump = new Lump(wallLump.nameAsString() + 'D', darkendData);
				} else {
					newWallLump = new Lump(wallLump.name(), wallLump.data());
				}
				newbb.putShort(NumberUtils.toShort(index | (wallResourceNumber & 0x4000)));
				wadFile.setLump(index, newWallLump);
				index++;
			}
		}

		wadFile.setLump(MyWallList, new Lump(wallListLump.name(), newbb.array()));
	}

	private void swapShortEndianness(int lumpnum) {
		Lump lump = wadFile.getLump(lumpnum);
		swapShortEndianness(lump.data(), lump.length() / 2);
	}

	private void swapShortEndianness(byte[] data, int n) {
		for (int i = 0; i < n; i++) {
			byte t = data[i * 2 + 0];
			data[i * 2 + 0] = data[i * 2 + 1];
			data[i * 2 + 1] = t;
		}
	}

	private void makeRaw(int lumpnum) {
		Lump lump = wadFile.getLump(lumpnum);
		assert lump.length() == 2 + 2 + 320 * 200;

		Lump rawLump = new Lump(lump.name(), Arrays.copyOfRange(lump.data(), 4, 64004));
		wadFile.setLump(lumpnum, rawLump);
	}

	private void resize(int lumpnum) {
		Lump lump = wadFile.getLump(lumpnum);
		byte[] data = DLZSS(lump, 4);
		byte[] resizedData = resize(data, 320, 200);
		Lump resizedLump = new Lump(lump.name(), resizedData);
		wadFile.setLump(lumpnum, resizedLump);
	}

	private byte[] resize(byte[] data, int newWidth, int newHeight) {
		ByteBuffer bb = ByteBuffer.wrap(data);
		short oldWidth = bb.getShort();
		short oldHeight = bb.getShort();

		ByteBuffer newBb = ByteBuffer.allocate(2 + 2 + newWidth * newHeight);
		newBb.order(ByteOrder.LITTLE_ENDIAN);
		newBb.putShort(NumberUtils.toShort(newWidth));
		newBb.putShort(NumberUtils.toShort(newHeight));

		int DXI = ((oldWidth - 1) << 7) / newWidth;
		int DYI = ((oldHeight - 1) << 7) / newHeight;

		int dc_y = 0;
		for (int h = 0; h < newHeight; h++) {
			int dc_x = 0;
			for (int w = 0; w < newWidth; w++) {
				newBb.put(data[2 + 2 + (dc_y >> 7) * oldWidth + (dc_x >> 7)]);
				dc_x += DXI;
			}
			dc_y += DYI;
		}

		return newBb.array();
	}

	private void decompress(int lumpnum) {
		Lump lump = wadFile.getLump(lumpnum);
		Lump decompressedLump = new Lump(lump.name(), DLZSS(lump, 4));
		wadFile.setLump(lumpnum, decompressedLump);
	}

	private byte[] DLZSS(Lump srcLump, int sizeOfLengthField) {
		ByteBuffer src = ByteBuffer.wrap(srcLump.data());

		int length;
		switch (sizeOfLengthField) {
		case 0 -> {
			src.order(ByteOrder.LITTLE_ENDIAN);
			length = 128 * 128;
		}
		case 2 -> {
			src.order(ByteOrder.LITTLE_ENDIAN);
			length = src.getShort();
		}
		case 4 -> {
			length = src.getInt();
			src.order(ByteOrder.LITTLE_ENDIAN);
		}
		default -> throw new IllegalArgumentException("Unknown size of length field: " + sizeOfLengthField);
		}

		ByteBuffer dest = ByteBuffer.allocate(length);
		ByteBuffer backPtr = ByteBuffer.wrap(dest.array());

		int bitBucket = NumberUtils.toInt(src.get()) | 0x100;
		do {
			if ((bitBucket & 1) == 1) {
				dest.put(src.get());
				--length;
			} else {
				int runCount = src.getShort();
				int fun = 0x1000 - (runCount & 0xfff);
				backPtr.position(dest.position() - fun);
				runCount = ((runCount >> 12) & 0x0f) + 3;
				if (length >= runCount) {
					length -= runCount;
				} else {
					length = 0;
				}
				do {
					dest.put(backPtr.get());
				} while (--runCount != 0);
			}
			bitBucket >>= 1;
			if (bitBucket == 1) {
				if (length != 0) {
					bitBucket = NumberUtils.toInt(src.get()) | 0x100;
				}
			}
		} while (length != 0);

		return dest.array();
	}

	/**
	 * @see <a href=
	 *      "https://github.com/fuzziqersoftware/resource_dasm/blob/master/src/DataCodecs/SoundMusicSys-LZSS.cc">resource_dasm</a>
	 * @param data
	 * @return
	 */
	private byte[] decompressSoundMusicSysLzss(byte[] data) {
		ByteBuffer r = ByteBuffer.wrap(data);
		int size = r.getInt();
		ByteBuffer ret = ByteBuffer.allocate(size);
		int retsize = 0;

		for (;;) {
			if (r.position() == data.length) {
				return ret.array();
			}
			byte control_bits = r.get();

			for (byte control_mask = 1; control_mask != 0; control_mask <<= 1) {
				if ((control_bits & control_mask) != 0) {
					if (r.position() == data.length) {
						return ret.array();
					}
					ret.put(r.get());
					retsize++;
				} else {
					if (r.position() >= data.length - 1) {
						return ret.array();
					}
					short params = r.getShort();

					int copy_offset = retsize - ((1 << 12) - (params & 0x0fff));
					int count = ((params >> 12) & 0x0f) + 3;
					int copy_end_offset = copy_offset + count;

					for (; copy_offset != copy_end_offset; copy_offset++) {
						ret.put(ret.get(copy_offset));
						retsize++;
					}
				}
			}
		}
	}

	private record PcSpeaker(String macName, String lumpName, String fileName) {
	}

	private void addPcSpeakerSoundEffects() {
		List<PcSpeaker> sfxs = List.of( //
				new PcSpeaker("SND_THROWSWITCH", "LEVELDON", "040-LEVELDONE"), /* Throw end level switch */
				new PcSpeaker("SND_GETKEY", "GETKEYS", "012-GETKEY"), /* Pick up a key */
				new PcSpeaker("SND_BONUS", "BONUS2", "036-BONUS2"), /* Score ding */
				new PcSpeaker("SND_OPENDOOR", "OPENDOOR", "018-OPENDOOR"), /* Open a door */
				new PcSpeaker("SND_DOGBARK", "DOGATTAC", "068-DOGATTACK"), /* Dog bite */
				new PcSpeaker("SND_DOGDIE", "DOGDEATH", "010-DOGDEATH"), /* Dog die */
				new PcSpeaker("SND_ESEE", "HALT", "021-HALT"), /* Ahtung! */
				new PcSpeaker("SND_ESEE2", "SCHUTZAD", "051-SCHUTZAD"), /* Halt! */
				new PcSpeaker("SND_EDIE", "DEATHSCR", "029-DEATHSCREAM1"), /* Nazi died */
				new PcSpeaker("SND_EDIE2", "DEATHSCR", "022-DEATHSCREAM2"), /* Nazi died 2 */
				new PcSpeaker("SND_BODYFALL", "BODYFALL", null), /* Body hit the ground */
				new PcSpeaker("SND_PAIN", "PAIN", null), /* Hit bad guy */
				new PcSpeaker("SND_GETAMMO", "GETAMMO", "031-GETAMMO"), /* Pick up ammo */
				new PcSpeaker("SND_KNIFE", "ATKKNIFE", "023-ATKKNIFE"), /* Knife attack */
				new PcSpeaker("SND_GUNSHT", "ATKPISTO", "024-ATKPISTOL"), /* 45 Shoot */
				new PcSpeaker("SND_MGUN", "ATKMACHI", "026-ATKMACHINEGUN"), /* Sub machine gun */
				new PcSpeaker("SND_CHAIN", "ATKGATLI", "011-ATKGATLING"), /* Chain gun */
				new PcSpeaker("SND_FTHROW", "FLAMETHR", "069-FLAMETHROWER"), /* Flame thrower */
				new PcSpeaker("SND_ROCKET", "MISSILEF", "085-MISSILEFIRE"), /* Rocket launcher */
				new PcSpeaker("SND_PWALL", "PUSHWALL", "046-PUSHWALL"), /* Start pushwall */
				new PcSpeaker("SND_PWALL2", "PWALL2", null), /* Stop pushwall */
				new PcSpeaker("SND_GUTEN", "GUTENTAG", "055-GUTENTAG"), /* Guten tag */
				new PcSpeaker("SND_SHIT", "SCHEIST", "057-SCHEIST"), /* Shit! */
				new PcSpeaker("SND_HEAL", "HEALTH1", "033-HEALTH1"), /* Healed a bit */
				new PcSpeaker("SND_THUMBSUP", "GETGATLI", "038-GETGATLING"), /* You stud you! */
				new PcSpeaker("SND_EXTRA", "BONUS1UP", "044-BONUS1UP"), /* Extra guy */
				new PcSpeaker("SND_OUCH1", "OUCH1", null), /* BJ has beed injured */
				new PcSpeaker("SND_OUCH2", "OUCH2", null), /* Second sound */
				new PcSpeaker("SND_PDIE", "PLAYERDA", "009-PLAYERDEATh"), /* BJ has died */
				new PcSpeaker("SND_HITWALL", "DONOTHIN", "020-DONOTHING"), /* Tried to open a wall */
				new PcSpeaker("SND_KNIFEMISS", "ATKKNIFE", "023-ATKKNIFE"), /* Knife missed */
				new PcSpeaker("SND_BIGGUN", "BOSSFIRE", "059-BOSSFIRE"), /* Boss's gun */
				new PcSpeaker("SND_COMEHERE", "GUTENTAG", "055-GUTENTAG"), /* Come here! */
				new PcSpeaker("SND_ESEE3", "HALT", "021-HALT"), /* Nazi sees you */
				new PcSpeaker("SND_ESEE4", "SPION", "066-SPION"), /* Nazi sees you */
				/* UNUSED */ new PcSpeaker("SND_OK", "OK", null), /* Hit start game */
				/* UNUSED */ new PcSpeaker("SND_MENU", "MENU", null), /* New game menu */
				new PcSpeaker("SND_HITLERSEE", "DIE", "053-DIE"), /* Hitler sees you */
				new PcSpeaker("SND_SHITHEAD", "SCHABBSH", "064-SCHABBSHA"), /* Big boss sees you */
				new PcSpeaker("SND_BOOM", "MISSILEH", "086-MISSILEHIT"), /* Explosion */
				new PcSpeaker("SND_LOCKEDDOOR", "DONOTHIN", "020-DONOTHING"), /* Locked door */
				new PcSpeaker("SND_MECHSTEP", "MECHSTEP", "070-MECHSTEP") /* Mech step */
		);

		byte[] empty = { 1, 0, 0, 0, 0, 0, 0 };

		int i = 151;
		System.out.println("static const Word priorities[NUMSOUNDS] = {");
		System.out.println("\t0, // SND_NOSOUND");
		for (PcSpeaker sfx : sfxs) {
			byte[] audiot = sfx.fileName != null ? getBytes("PC Sounds/PC Sound-" + sfx.fileName + "SND.pc") : empty;
			ByteBuffer bb = ByteBuffer.wrap(audiot);
			bb.order(ByteOrder.LITTLE_ENDIAN);
			int length = bb.getInt();
			short priority = bb.getShort();
			byte[] data = new byte[length];
			bb.get(data);
			System.out.println("\t" + priority + ", // " + sfx.macName);
			Lump lump = new Lump(sfx.lumpName, data);
			wadFile.setLump(i, lump);
			i++;
		}
		System.out.println("};");
		System.out.println();
	}

	/**
	 * @see <a href=
	 *      "https://www.vgmpf.com/Wiki/index.php/Wolfenstein_3D_(MAC)">Video Game
	 *      Music Preservation Foundation - Wolfenstein 3D (MAC)</a>
	 */
	private void addMusic(Episode episode) {
		Lump genmidi = new Lump("GENMIDI", getBytes("GENMIDI.OP2"));
		Lump title = new Lump("Title", getBytes("01 - Title.mid"));
		Lump plodding = new Lump("Plodding", getBytes("02 - Plodding.mid"));
		Lump unleashed = new Lump("Unleashed", getBytes("07 - Unleashed.mid"));

		wadFile.setLump(57, createTimbreBank(genmidi));

		wadFile.setLump(63, plodding);
		wadFile.setLump(64, title);

		wadFile.setLump(66, unleashed);
		wadFile.setLump(68, title);

		if (episode == Episode.SECOND_ENCOUNTER) {
			Lump rocked = new Lump("Rocked", getBytes("03 - Rocked.mid"));
			Lump original = new Lump("Original", getBytes("04 - Original.mid"));
			Lump doom = new Lump("Doom", getBytes("05 - Doom.mid"));
			Lump grunge = new Lump("Grunge", getBytes("06 - Grunge.mid"));

			wadFile.setLump(58, rocked);
			wadFile.setLump(59, original);
			wadFile.setLump(60, doom);
			wadFile.setLump(65, grunge);
		}

		Lump songList = wadFile.getLump(rSongList);
		for (int i = 0; i < songList.length(); i += 2) {
			songList.data()[i] = NumberUtils.toByte(NumberUtils.toInt(songList.data()[i]) - 70);
		}
	}

	private Lump createTimbreBank(Lump genmidi) {
		byte[] timbres = genmidi.data();
		byte[] tmb = new byte[13 * 256];

		for (int i = 0; i < 128; i++) {
			tmb[i * 13 + 0] = timbres[8 + i * 36 + 4 + 0];
			tmb[i * 13 + 1] = timbres[8 + i * 36 + 4 + 7];
			tmb[i * 13 + 2] = NumberUtils.toByte(
					NumberUtils.toInt(timbres[8 + i * 36 + 4 + 4]) | NumberUtils.toInt(timbres[8 + i * 36 + 4 + 5]));
			tmb[i * 13 + 3] = NumberUtils.toByte(NumberUtils.toInt(timbres[8 + i * 36 + 4 + 11]) & 192);
			tmb[i * 13 + 4] = timbres[8 + i * 36 + 4 + 1];
			tmb[i * 13 + 5] = timbres[8 + i * 36 + 4 + 8];
			tmb[i * 13 + 6] = timbres[8 + i * 36 + 4 + 2];
			tmb[i * 13 + 7] = timbres[8 + i * 36 + 4 + 9];
			tmb[i * 13 + 8] = timbres[8 + i * 36 + 4 + 3];
			tmb[i * 13 + 9] = timbres[8 + i * 36 + 4 + 10];
			tmb[i * 13 + 10] = timbres[8 + i * 36 + 4 + 6];
			int transpose = NumberUtils.toInt(timbres[8 + i * 36 + 4 + 14]) + 12;
			if (transpose > 255) {
				transpose &= 255;
			}
			tmb[i * 13 + 11] = NumberUtils.toByte(transpose);
			tmb[i * 13 + 12] = 0;
		}

		for (int i = 128; i < 175; i++) {
			tmb[(i + 35) * 13 + 0] = timbres[8 + i * 36 + 4 + 0];
			tmb[(i + 35) * 13 + 1] = timbres[8 + i * 36 + 4 + 7];
			tmb[(i + 35) * 13 + 2] = NumberUtils.toByte(
					NumberUtils.toInt(timbres[8 + i * 36 + 4 + 4]) | NumberUtils.toInt(timbres[8 + i * 36 + 4 + 5]));
			tmb[(i + 35) * 13 + 3] = NumberUtils.toByte(NumberUtils.toInt(timbres[8 + i * 36 + 4 + 11]) & 192);
			tmb[(i + 35) * 13 + 4] = timbres[8 + i * 36 + 4 + 1];
			tmb[(i + 35) * 13 + 5] = timbres[8 + i * 36 + 4 + 8];
			tmb[(i + 35) * 13 + 6] = timbres[8 + i * 36 + 4 + 2];
			tmb[(i + 35) * 13 + 7] = timbres[8 + i * 36 + 4 + 9];
			tmb[(i + 35) * 13 + 8] = timbres[8 + i * 36 + 4 + 3];
			tmb[(i + 35) * 13 + 9] = timbres[8 + i * 36 + 4 + 10];
			tmb[(i + 35) * 13 + 10] = timbres[8 + i * 36 + 4 + 6];
			tmb[(i + 35) * 13 + 11] = NumberUtils.toByte(
					NumberUtils.toInt(timbres[8 + i * 36 + 3]) + NumberUtils.toInt(timbres[8 + i * 36 + 4 + 14]) + 12);
			tmb[(i + 35) * 13 + 12] = 0;
		}

		return new Lump(genmidi.name(), tmb);
	}

	private void processCompressedSound(Type type) {
		List<Resource> soundResources = type.resourceList().stream().filter(r -> r.id() < 10000).toList();

		for (Resource resource : soundResources) {
			byte[] data = decompressSoundMusicSysLzss(resource.getData());
			deltaDecompress(data);
			processSound(resource, data);
		}
	}

	private void processUncompressedSound(Type type) {
		List<Resource> soundResources = type.resourceList().stream().filter(r -> r.id() < 10000).toList();

		for (Resource resource : soundResources) {
			processSound(resource, resource.getData());
		}
	}

	private void processSound(Resource resource, byte[] data) {
		assertSoundData(data);
		byte[] newData = Arrays.copyOfRange(data, 42, Math.min(data.length, 65504 + 42));
		wadFile.setLump(resource.id() - 50, new Lump(resource.getName(), newData));
	}

	private void deltaDecompress(byte[] data) {
		int sample = NumberUtils.toInt(data[0]);
		for (int i = 1; i < data.length; i++) {
			sample += NumberUtils.toInt(data[i]);
			sample &= 255;
			data[i] = NumberUtils.toByte(sample);
		}
	}

	private void assertSoundData(byte[] data) {
		ByteBuffer bb = ByteBuffer.wrap(data);
		short formatType = bb.getShort();
		short numberOfDataTypes = bb.getShort();
		short firstDataType = bb.getShort();
		int initializationOption = bb.getInt();
		short numberOfSoundCommands = bb.getShort();
		short bufferCmd = bb.getShort();
		short param1 = bb.getShort();
		int param2 = bb.getInt();
		int pointerToData = bb.getInt();
		int numberOfBytesInSample = bb.getInt();
		int samplingRate = bb.getInt();
		int loopPointStart = bb.getInt();
		int loopPointEnd = bb.getInt();
		byte standardSampleEncoding = bb.get();
		byte baseFrequency = bb.get();

		assertEquals(1, formatType);
		assertEquals(1, numberOfDataTypes);
		assertEquals(5, firstDataType);
		assertEquals(0xa0, initializationOption);
		assertEquals(1, numberOfSoundCommands);
		assertEquals(0x8051, bufferCmd);
		assertEquals(0, param1);
		assertEquals(0x14, param2);
		assertEquals(0, pointerToData);
		assertEquals(data.length - 42, numberOfBytesInSample);

		assert samplingRate == 0x56ee0000 || samplingRate == 0x56ee8b9f : "Actual " + Integer.toHexString(samplingRate);

		assertEquals(data.length - 44, loopPointStart);
		assertEquals(data.length - 43, loopPointEnd);
		assertEquals(0, standardSampleEncoding);
		assertEquals(0x3c, baseFrequency);
	}

	private void assertEquals(int expectedValueAsInt, byte actualValue) {
		byte expectedValue = NumberUtils.toByte(expectedValueAsInt);
		assert expectedValue == actualValue;
	}

	private void assertEquals(int expectedValueAsInt, short actualValue) {
		short expectedValue = NumberUtils.toShort(expectedValueAsInt);
		assert expectedValue == actualValue : "Expected " + Integer.toHexString(expectedValueAsInt) + ", actual "
				+ Integer.toHexString(NumberUtils.toInt(actualValue));
	}

	private void assertEquals(int expectedValue, int actualValue) {
		assert expectedValue == actualValue
				: "Expected " + Integer.toHexString(expectedValue) + ", actual " + Integer.toHexString(actualValue);
	}
}
