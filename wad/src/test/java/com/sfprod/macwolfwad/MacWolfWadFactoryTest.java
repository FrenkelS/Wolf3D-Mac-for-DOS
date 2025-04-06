package com.sfprod.macwolfwad;

import static com.sfprod.macwolfwad.ResourceFileTest.assumeFileExists;
import static org.junit.jupiter.api.Assertions.assertEquals;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.zip.CRC32;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.CsvSource;

/**
 * This class tests {@link MacWolfWadFactory}
 */
class MacWolfWadFactoryTest {

	@Test
	void createWadFirstEncounter() throws Exception {
		Episode episode = Episode.FIRST_ENCOUNTER;

		MacWolfWadFactory macWolfWadFactory = new MacWolfWadFactory();
		macWolfWadFactory.createWad(episode);

		CRC32 crc32 = new CRC32();
		crc32.update(Files.readAllBytes(Path.of("target", episode.getOutputFilename())));

		assertEquals("1103622D", Long.toHexString(crc32.getValue()).toUpperCase());
	}

	@Test
	void createWadSecondEncounter() throws Exception {
		Episode episode = Episode.SECOND_ENCOUNTER;

		assumeFileExists(episode.getInputFilename());

		MacWolfWadFactory macWolfWadFactory = new MacWolfWadFactory();
		macWolfWadFactory.createWad(episode);

		CRC32 crc32 = new CRC32();
		crc32.update(Files.readAllBytes(Path.of("target", episode.getOutputFilename())));

		assertEquals("FA11B20B", Long.toHexString(crc32.getValue()).toUpperCase());
	}

	@ParameterizedTest
	@CsvSource({ //
			"1 Escape From Wolfenstein, BED89984", //
			"2 Operation-Eisenfaust, 71710DDA", //
			"'3 Die, Führer, Die!', 7DE52050", //
			"4 A Dark Secret, 754DF1FA", //
			"5 Trail of the Madman, E1D5D0E8", //
			"6 Confrontation, 52284BCE" })
	void createMapWadThirdEncounter(String inputFilename, String checksum) throws Exception {
		assumeFileExists(inputFilename);

		String outputFilename = "MW3E" + inputFilename.charAt(0) + ".WAD";

		MacWolfWadFactory macWolfWadFactory = new MacWolfWadFactory();
		macWolfWadFactory.createMapWad(inputFilename, outputFilename);

		CRC32 crc32 = new CRC32();
		crc32.update(Files.readAllBytes(Path.of("target", outputFilename)));

		assertEquals(checksum, Long.toHexString(crc32.getValue()).toUpperCase());

	}
}
