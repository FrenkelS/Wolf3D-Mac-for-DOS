package com.sfprod.macwolfwad;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Optional;
import java.util.stream.Collectors;
import java.util.zip.CRC32;

import org.junit.jupiter.api.Test;

/**
 * This class tests {@link MacWolfWadFactory}
 */
class MacWolfWadFactoryTest {

	private final MacWolfWadFactory macWolfWadFactory = new MacWolfWadFactory();
	private final Map<String, ResourceFile> resources = macWolfWadFactory.identifyInput();

	@Test
	void createFirstAndSecondEncounter() throws Exception {
		Map<String, ResourceFile> firstAndSecondEncounter = findResourceFiles(ContentType.FIRST_ENCOUNTER,
				ContentType.SECOND_ENCOUNTER);

		assertFalse(firstAndSecondEncounter.isEmpty());

		for (Entry<String, ResourceFile> entry : firstAndSecondEncounter.entrySet()) {
			String inputFilename = entry.getKey();
			ResourceFile encounter = entry.getValue();
			macWolfWadFactory.createWad(inputFilename, encounter);

			checkCrc(encounter);
		}
	}

	@Test
	void createThirdEncounter() throws Exception {
		Optional<Entry<String, ResourceFile>> optionalThirdEncounter = findResourceFile(ContentType.THIRD_ENCOUNTER);
		Optional<Entry<String, ResourceFile>> optionalSecondEncounterMaps = findResourceFile(
				ContentType.SECOND_ENCOUNTER_MAPS);
		if (optionalThirdEncounter.isPresent() && optionalSecondEncounterMaps.isPresent()) {
			Entry<String, ResourceFile> thirdEncounterEntry = optionalThirdEncounter.get();
			Entry<String, ResourceFile> secondEncounterMapsEntry = optionalSecondEncounterMaps.get();
			String inputFilename = thirdEncounterEntry.getKey() + " and " + secondEncounterMapsEntry.getKey();
			ResourceFile thirdEncounterResourceFile = thirdEncounterEntry.getValue();
			macWolfWadFactory.createWad(inputFilename, thirdEncounterResourceFile, secondEncounterMapsEntry.getValue());

			checkCrc(thirdEncounterResourceFile);
		} else if (optionalThirdEncounter.isPresent()) {
			System.out.println("Third Encounter found in " + optionalThirdEncounter.get().getKey()
					+ ", but no Second Encounter maps");
		} else if (optionalSecondEncounterMaps.isPresent()) {
			System.out.println("Second Encounter maps found in " + optionalSecondEncounterMaps.get().getKey()
					+ ", but no Third Encounter");
		}
	}

	@Test
	void createThirddEncounterEpisodes() throws Exception {
		Map<String, ResourceFile> episodes = findResourceFiles( //
				ContentType.THIRD_ENCOUNTER_EPISODE1_MAPS, //
				ContentType.THIRD_ENCOUNTER_EPISODE2_MAPS, //
				ContentType.THIRD_ENCOUNTER_EPISODE3_MAPS, //
				ContentType.THIRD_ENCOUNTER_EPISODE4_MAPS, //
				ContentType.THIRD_ENCOUNTER_EPISODE5_MAPS, //
				ContentType.THIRD_ENCOUNTER_EPISODE6_MAPS);
		for (Entry<String, ResourceFile> episodeEntry : episodes.entrySet()) {
			ResourceFile episode = episodeEntry.getValue();
			macWolfWadFactory.createMapWad(episodeEntry.getKey(), episode,
					episode.getContentType().getOutputFilename());

			checkCrc(episode);
		}
	}

	private void checkCrc(ResourceFile encounter) throws Exception {
		CRC32 crc32 = new CRC32();
		crc32.update(Files.readAllBytes(Path.of("target", encounter.getContentType().getOutputFilename())));

		assertEquals(encounter.getContentType().getCrc32(), Long.toHexString(crc32.getValue()).toUpperCase());
	}

	@Test
	void createUnknownMaps() {
		Map<String, ResourceFile> unknownMaps = findResourceFiles(ContentType.UNKNOWN_MAPS);
		int i = 1;
		for (Entry<String, ResourceFile> unknownMap : unknownMaps.entrySet()) {
			macWolfWadFactory.createMapWad(unknownMap.getKey(), unknownMap.getValue(), "MAPS" + i + ".WAD");
			i++;
		}
	}

	private Optional<Entry<String, ResourceFile>> findResourceFile(ContentType contentType) {
		return resources.entrySet().stream().filter(e -> e.getValue().getContentType() == contentType).findAny();
	}

	private Map<String, ResourceFile> findResourceFiles(ContentType... contentTypes) {
		return resources.entrySet().stream()
				.filter(e -> Arrays.asList(contentTypes).contains(e.getValue().getContentType()))
				.collect(Collectors.toMap(Entry::getKey, Entry::getValue));
	}
}
