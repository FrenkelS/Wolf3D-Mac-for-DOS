package com.sfprod.macwolfwad;

import java.nio.charset.StandardCharsets;
import java.util.Arrays;

record Lump(byte[] name, byte[] data) {

	Lump {
		name = Arrays.copyOf(name, 8);
	}

	Lump(String nameAsString, byte[] data) {
		this(nameAsString.getBytes(StandardCharsets.US_ASCII), data);
	}

	String nameAsString() {
		return new String(name).trim();
	}

	int length() {
		return data.length;
	}

	boolean isEmpty() {
		return data.length == 0;
	}
}