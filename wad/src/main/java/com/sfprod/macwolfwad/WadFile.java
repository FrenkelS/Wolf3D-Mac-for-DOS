package com.sfprod.macwolfwad;

import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

class WadFile {

	private static final Lump EMPTY_LUMP = new Lump("UNUSED", new byte[] {});

	private final List<Lump> lumps;

	WadFile(int count) {
		this.lumps = new ArrayList<>(count);

		for (int i = 0; i < count; i++) {
			lumps.add(EMPTY_LUMP);
		}
	}

	void saveWadFile(String outputFilename) {
		int filepos = 4 + 4 + 4 + lumps.size() * (4 + 4 + 8);
		int filesize = filepos + lumps.stream().mapToInt(Lump::length).sum();

		ByteBuffer byteBuffer = ByteBuffer.allocate(filesize);
		byteBuffer.order(ByteOrder.LITTLE_ENDIAN);

		byteBuffer.put("IWAD".getBytes(StandardCharsets.US_ASCII));
		byteBuffer.putInt(lumps.size());
		byteBuffer.putInt(4 + 4 + 4);

		// WAD lump merging
		Map<String, Integer> duplicateDataMap = new HashMap<>();
		int duplicateDataCount = 0;
		for (int lumpnum = 0; lumpnum < lumps.size(); lumpnum++) {
			Lump lump = lumps.get(lumpnum);
			if (lump.length() == 0) {
				byteBuffer.putInt(0);
			} else {
				String key = Arrays.toString(lump.data());
				if (duplicateDataMap.containsKey(key)) {
					int previousfilepos = duplicateDataMap.get(key);
					byteBuffer.putInt(previousfilepos);
					Lump newLump = new Lump(lump.name(), new byte[] {});
					lumps.set(lumpnum, newLump);
					duplicateDataCount++;
				} else {
					duplicateDataMap.put(key, filepos);
					byteBuffer.putInt(filepos);
					filepos += lump.length();
				}
			}

			byteBuffer.putInt(lump.length());
			byteBuffer.put(lump.name());
		}
		System.out.println("Removed " + duplicateDataCount + " duplicate lumps");

		for (Lump lump : lumps) {
			byteBuffer.put(lump.data());
		}

		Path path = Path.of("target", outputFilename);
		int filesizeWithoutDuplicates = 4 + 4 + 4 + lumps.size() * (4 + 4 + 8)
				+ lumps.stream().mapToInt(Lump::length).sum();

		try {
			Files.write(path, Arrays.copyOf(byteBuffer.array(), filesizeWithoutDuplicates));
		} catch (IOException e) {
			throw new UncheckedIOException(e);
		}

		System.out.println("WAD file of size " + filesizeWithoutDuplicates + " written to " + path.toAbsolutePath());
	}

	int getLumpCount() {
		return lumps.size();
	}

	void removeLump(int lumpnum) {
		lumps.set(lumpnum, EMPTY_LUMP);
	}

	Lump getLump(int lumpnum) {
		return lumps.get(lumpnum);
	}

	void setLump(int lumpnum, Lump lump) {
		lumps.set(lumpnum, lump);
	}

}
