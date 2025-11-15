// This code is based on code found at https://github.com/meetanthony/crcjava

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Anton Isakov http://crccalc.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
package com.sfprod.utils;

/**
 * A class that can be used to compute the CRC-16/XMODEM of a data stream.
 *
 */
public class CRC16Xmodem {

	private short crc = 0;

	public void update(byte[] bytes) {
		for (byte b : bytes) {
			byte hiByte = (byte) (crc >> 8);
			byte loByte = (byte) (crc & 0xff);

			byte xoredByte = (byte) (hiByte ^ b);
			short s = processByte(xoredByte);

			crc = (short) (s ^ (loByte << 8));
		}
	}

	private short processByte(byte b) {
		short r = b;
		r <<= 8;

		for (int i = 0; i < 8; i++) {
			short poly = (short) ((r & 0x8000) == 0 ? 0 : 0x1021);
			r = (short) ((r << 1) ^ poly);
		}

		return r;
	}

	public short getValue() {
		return crc;
	}

}
