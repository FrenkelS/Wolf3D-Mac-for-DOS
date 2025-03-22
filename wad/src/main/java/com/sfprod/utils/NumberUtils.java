package com.sfprod.utils;

public interface NumberUtils {

	static byte toByte(int i) {
		if (!(0 <= i && i < 256)) {
			throw new IllegalArgumentException(i + " doesn't fit in a byte");
		}

		return (byte) i;
	}

	/**
	 * Convert signed byte to an unsigned int.
	 *
	 * @param b
	 * @return
	 */
	static int toInt(byte b) {
		return Byte.toUnsignedInt(b);
	}

	static int toInt(short s) {
		return Short.toUnsignedInt(s);
	}

	static short toShort(int i) {
		if (!(-32768 <= i && i < 65536)) {
			throw new IllegalArgumentException(i + " doesn't fit in a short");
		}

		return (short) i;
	}
}
