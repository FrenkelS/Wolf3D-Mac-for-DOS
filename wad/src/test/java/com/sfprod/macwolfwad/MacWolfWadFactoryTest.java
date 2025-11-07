package com.sfprod.macwolfwad;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.zip.CRC32;

import org.junit.jupiter.api.Test;

/**
 * This class tests {@link MacWolfWadFactory}
 */
class MacWolfWadFactoryTest {

	@Test
	void identifyInput() throws Exception {
		MacWolfWadFactory macWolfWadFactory = new MacWolfWadFactory();
		Map<String, ResourceFile> resources = macWolfWadFactory.identifyInput();

		List<ResourceFile> firstAndSecondEncounter = resources.values().stream()
				.filter(r -> r.getContentType() == ContentType.FIRST_ENCOUNTER
						|| r.getContentType() == ContentType.SECOND_ENCOUNTER)
				.toList();

		assertFalse(firstAndSecondEncounter.isEmpty());

		for (ResourceFile encounter : firstAndSecondEncounter) {
			macWolfWadFactory.createWad(encounter);

			CRC32 crc32 = new CRC32();
			crc32.update(Files.readAllBytes(Path.of("target", encounter.getContentType().getOutputFilename())));

			assertEquals(encounter.getContentType().getCrc32(), Long.toHexString(crc32.getValue()).toUpperCase());
		}

		Optional<ResourceFile> optionalThirdEncounter = findResourceFile(resources, ContentType.THIRD_ENCOUNTER);
		Optional<ResourceFile> optionalSecondEncounterMaps = findResourceFile(resources,
				ContentType.SECOND_ENCOUNTER_MAPS);
		if (optionalThirdEncounter.isPresent() && optionalSecondEncounterMaps.isPresent()) {
			ResourceFile thirdEncounter = optionalThirdEncounter.get();
			macWolfWadFactory.createWad(thirdEncounter, optionalSecondEncounterMaps.get());

			CRC32 crc32 = new CRC32();
			crc32.update(Files.readAllBytes(Path.of("target", thirdEncounter.getContentType().getOutputFilename())));

			assertEquals(thirdEncounter.getContentType().getCrc32(), Long.toHexString(crc32.getValue()).toUpperCase());
		} else if (optionalThirdEncounter.isPresent()) {
			System.out.println("Third Encounter found, but no Second Encounter maps");
		} else if (optionalSecondEncounterMaps.isPresent()) {
			System.out.println("Second Encounter maps found, but no Third Encounter");
		}

		List<ResourceFile> thirdEncounterEpisodes = resources.values().stream()
				.filter(r -> r.getContentType() == ContentType.THIRD_ENCOUNTER_EPISODE1_MAPS
						|| r.getContentType() == ContentType.THIRD_ENCOUNTER_EPISODE2_MAPS
						|| r.getContentType() == ContentType.THIRD_ENCOUNTER_EPISODE3_MAPS
						|| r.getContentType() == ContentType.THIRD_ENCOUNTER_EPISODE4_MAPS
						|| r.getContentType() == ContentType.THIRD_ENCOUNTER_EPISODE5_MAPS
						|| r.getContentType() == ContentType.THIRD_ENCOUNTER_EPISODE6_MAPS)
				.toList();
		for (ResourceFile thirdEncounterEpisode : thirdEncounterEpisodes) {
			macWolfWadFactory.createMapWad(thirdEncounterEpisode,
					thirdEncounterEpisode.getContentType().getOutputFilename());

			CRC32 crc32 = new CRC32();
			crc32.update(
					Files.readAllBytes(Path.of("target", thirdEncounterEpisode.getContentType().getOutputFilename())));

			assertEquals(thirdEncounterEpisode.getContentType().getCrc32(),
					Long.toHexString(crc32.getValue()).toUpperCase());
		}

		List<ResourceFile> unknownMaps = resources.values().stream()
				.filter(r -> r.getContentType() == ContentType.UNKNOWN_MAPS).toList();
		int i = 1;
		for (ResourceFile unknownMap : unknownMaps) {
			macWolfWadFactory.createMapWad(unknownMap, "MAPS" + i + ".WAD");
			i++;
		}
	}

	private Optional<ResourceFile> findResourceFile(Map<String, ResourceFile> resources, ContentType contentType) {
		return resources.values().stream().filter(r -> r.getContentType() == contentType).findAny();
	}
}
