package com.sfprod.macwolfwad;

import static org.junit.jupiter.api.Assertions.assertEquals;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.zip.CRC32;

import org.junit.jupiter.api.Test;

/**
 * This class tests {@link MacWolfWadFactory}
 */
class MacWolfWadFactoryTest {

	@Test
	void createWad() throws Exception {
		Episode episode = Episode.FIRST_ENCOUNTER;

		MacWolfWadFactory macWolfWadFactory = new MacWolfWadFactory();
		macWolfWadFactory.createWad(episode);

		CRC32 crc32 = new CRC32();
		crc32.update(Files.readAllBytes(Path.of("target", episode.getOutputFilename())));

		assertEquals("5C17472C", Long.toHexString(crc32.getValue()).toUpperCase());
	}
}
