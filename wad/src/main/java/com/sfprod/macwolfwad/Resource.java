package com.sfprod.macwolfwad;

class Resource {
	private final short id;
	private final short nameOffset;
	private final byte attributes;
	private final int dataOffset;

	private byte[] name = {};
	private byte[] data;

	Resource(short id, short nameOffset, byte attributes, int dataOffset) {
		this.id = id;
		this.nameOffset = nameOffset;
		this.attributes = attributes;
		this.dataOffset = dataOffset;
	}

	byte[] getName() {
		return name;
	}

	void setName(byte[] name) {
		this.name = name;
	}

	byte[] getData() {
		return data;
	}

	void setData(byte[] data) {
		this.data = data;
	}

	short id() {
		return id;
	}

	short nameOffset() {
		return nameOffset;
	}

	byte attributes() {
		return attributes;
	}

	int dataOffset() {
		return dataOffset;
	}

}
