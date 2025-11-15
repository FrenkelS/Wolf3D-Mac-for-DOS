package com.sfprod.utils;

import static org.junit.jupiter.api.Assertions.assertEquals;

import java.nio.charset.StandardCharsets;

import org.junit.jupiter.api.Test;

/**
 * This class tests {@link CRC16Xmodem}
 */
class CRC16XmodemTest {

	@Test
	void update() {
		CRC16Xmodem crc16xmodem = new CRC16Xmodem();
		crc16xmodem.update("123456789".getBytes(StandardCharsets.US_ASCII));
		assertEquals((short) 0x31C3, crc16xmodem.getValue());
	}

}
