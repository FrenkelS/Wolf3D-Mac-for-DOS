package com.sfprod.macwolfwad;

import java.nio.ByteBuffer;

import com.sfprod.utils.CRC16Xmodem;

/**
 * @see <a href=
 *      "https://ciderpress2.com/formatdoc/MacBinary-notes.html">MacBinary File
 *      Format</a>
 */
class MacBinaryFile {

	private final ResourceFile resourceFile;

	MacBinaryFile(byte[] bytes) {
		assert isMacBinary(bytes);

		ByteBuffer byteBuffer = ByteBuffer.wrap(bytes);
		byteBuffer.position(0x53);
		int dataForkLength = byteBuffer.getInt();
		int resourceForkLength = byteBuffer.getInt();

		int offset = 0x80 + ((dataForkLength + 0x7f) & 0xffffff80);

		byte[] resourceFileBytes = new byte[resourceForkLength];
		byteBuffer.position(offset);
		byteBuffer.get(resourceFileBytes);

		this.resourceFile = new ResourceFile(resourceFileBytes);
	}

	static boolean isMacBinary(byte[] bytes) {
		ByteBuffer byteBuffer = ByteBuffer.wrap(bytes);

		byte[] header = new byte[124];
		byteBuffer.get(header);

		short expectedCrc = byteBuffer.getShort();

		CRC16Xmodem crc16xmodem = new CRC16Xmodem();
		crc16xmodem.update(header);
		short actualCrc = crc16xmodem.getValue();

		return expectedCrc == actualCrc;
	}

	ResourceFile getResourceFile() {
		return resourceFile;
	}
}
