package com.sfprod.macwolfwad;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assumptions.assumeTrue;

import java.util.List;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.ValueSource;

class ResourceFileTest {

	private static final List<String> FIRST_ENCOUNTER_TYPES = List.of("vers", "BRGR", "FREF", "MBAR", "SMOD", "WOLF",
			"MDRV", "STR#", "INST", "Midi", "SONG", "proc", "prox", "cicn", "csnd", "icl4", "icl8", "ICN#", "ics#",
			"ics4", "ics8", "ALRT", "DITL", "DLOG", "PICT", "BNDL", "MENU", "mstr", "snd ", "hfdr", "hmnu", "TEXT",
			"CODE", "XREF", "DATA", "SIZE", "cfrg");

	private static final List<String> SECOND_ENCOUNTER_TYPES = List.of("BRGR", "FREF", "MENU", "MBAR", "WOLF", "STR#",
			"cicn", "icl4", "icl8", "ICN#", "ics#", "ics4", "ics8", "DLOG", "mstr", "csnd", "snd ", "INST", "Midi",
			"ALRT", "DITL", "PICT", "vers", "BNDL", "SMOD", "MDRV", "hfdr", "hmnu", "TEXT", "SONG", "CODE", "XREF",
			"DATA", "SIZE", "cfrg");

	@Test
	void getTypesFirstEncounter() {
		ResourceFile resourceFile = new ResourceFile(Episode.FIRST_ENCOUNTER);
		List<Type> types = resourceFile.getTypes();
		assertEquals(37, types.size());

		assertEquals(FIRST_ENCOUNTER_TYPES, types.stream().map(Type::type).toList());

		Type brgr = resourceFile.getType("BRGR");
		assertEquals(128, brgr.resourceCount());
		assertEquals(129, brgr.resourceList().size());
		assertEquals(593, brgr.calculateMaxId());

		Type csnd = resourceFile.getType("csnd");
		assertEquals(42, csnd.resourceCount());
		assertEquals(43, csnd.resourceList().size());

		Type snd = resourceFile.getType("snd ");
		assertEquals(2, snd.resourceCount());
		assertEquals(3, snd.resourceList().size());

		List<Resource> uncompressedSounds = snd.resourceList().stream().filter(r -> r.id() < 10000).toList();
		assertEquals(0, uncompressedSounds.size());
	}

	@Test
	void getTypesSecondEncounter() {
		Episode episode = Episode.SECOND_ENCOUNTER;

		assumeFileExists(episode.getInputFilename());

		ResourceFile resourceFile = new ResourceFile(episode);
		List<Type> types = resourceFile.getTypes();
		assertEquals(35, types.size());

		assertEquals(SECOND_ENCOUNTER_TYPES, types.stream().map(Type::type).toList());

		Type brgr = resourceFile.getType("BRGR");
		assertEquals(232, brgr.resourceCount());
		assertEquals(233, brgr.resourceList().size());
		assertEquals(601, brgr.calculateMaxId());

		Type csnd = resourceFile.getType("csnd");
		assertEquals(55, csnd.resourceCount());
		assertEquals(56, csnd.resourceList().size());

		Type snd = resourceFile.getType("snd ");
		assertEquals(5, snd.resourceCount());
		assertEquals(6, snd.resourceList().size());

		List<Resource> uncompressedSounds = snd.resourceList().stream().filter(r -> r.id() < 10000).toList();
		assertEquals(1, uncompressedSounds.size());

		Resource uncompressedSound = uncompressedSounds.getFirst();
		assertEquals(146, uncompressedSound.id());
		assertArrayEquals("ROCKET4.AIF".getBytes(), uncompressedSound.getName());
	}

	@Test
	void getTypesSecondEncounterLevels() {
		String filename = "Second Encounter (30 Levels)";
		assumeFileExists(filename);

		ResourceFile resourceFile = new ResourceFile(filename);
		List<Type> types = resourceFile.getTypes();
		assertEquals(2, types.size());

		assertEquals(List.of("BRGR", "PICT"), types.stream().map(Type::type).toList());

		Type brgr = resourceFile.getType("BRGR");
		assertEquals(32, brgr.resourceCount());
		assertEquals(33, brgr.resourceList().size());
		assertEquals(229, brgr.calculateMaxId());

		List<String> resourceNames = brgr.resourceList().stream().map(Resource::getName).map(String::new).toList();
		assertEquals(List.of("Sound effect list", "Map list", "Music List", "Map0", "Map1", "Map2"),
				resourceNames.subList(0, 6));
		assertEquals("Map 5", resourceNames.get(8));
		assertEquals("Map 10", resourceNames.get(13));
		assertEquals("Map 15", resourceNames.get(18));
		assertEquals("Map 20", resourceNames.get(23));
		assertEquals("Map 25", resourceNames.get(28));
	}

	@ParameterizedTest
	@ValueSource(strings = { "1 Escape From Wolfenstein", "2 Operation-Eisenfaust", "3 Die, Führer, Die!",
			"4 A Dark Secret", "5 Trail of the Madman", "6 Confrontation" })
	void getTypesThirdEncounterLevels(String filename) {
		assumeFileExists(filename);

		ResourceFile resourceFile = new ResourceFile(filename);
		List<Type> types = resourceFile.getTypes();
		assertEquals(2, types.size());

		assertEquals(List.of("BRGR", "PICT"), types.stream().map(Type::type).toList());

		Type brgr = resourceFile.getType("BRGR");
		assertEquals(15, brgr.resourceCount());
		assertEquals(16, brgr.resourceList().size());
		assertEquals(209, brgr.calculateMaxId());

		List<String> resourceNames = brgr.resourceList().stream().map(Resource::getName).map(String::new).toList();
		assertEquals(List.of("WallList", "GamePal", "Sound effect list", "Signature", "Map list"),
				resourceNames.subList(0, 5));

		if ("Map0".equals(resourceNames.get(5))) {
			assertEquals("Map0", resourceNames.get(5));
			assertEquals("Map1", resourceNames.get(6));
			assertEquals("Map2", resourceNames.get(7));
		} else {
			assertEquals("Map 0", resourceNames.get(5));
		}
		assertEquals("Map 5", resourceNames.get(10));
		assertEquals("Music List", resourceNames.get(15));
	}

	static void assumeFileExists(String filename) {
		assumeTrue(ResourceFileTest.class.getResource('/' + filename) != null);
	}
}
