package com.sfprod.macwolfwad;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

class AppleDoubleFile {

	private final ResourceFile resourceFile;

	AppleDoubleFile(byte[] bytes) {
		ByteBuffer byteBuffer = ByteBuffer.wrap(bytes);

		int magicNumber = byteBuffer.getInt();
		assert magicNumber == 0x00051607;

		int versionNumber = byteBuffer.getInt();
		assert versionNumber == 0x00020000;

		byte[] filler = new byte[16];
		byteBuffer.get(filler);

		short numberOfEntries = byteBuffer.getShort();
		assert numberOfEntries == 2;

		List<Entry> entries = new ArrayList<>(numberOfEntries);
		for (short i = 0; i < numberOfEntries; i++) {
			int entryID = byteBuffer.getInt();
			int offset = byteBuffer.getInt();
			int length = byteBuffer.getInt();
			entries.add(new Entry(entryID, offset, length));
		}

		assert entries.get(0).entryID == 0x00000009;

		Entry resourceFileEntry = entries.get(1);
		assert resourceFileEntry.entryID == 0x00000002;

		byte[] resourceFileBytes = new byte[resourceFileEntry.length];
		byteBuffer.position(resourceFileEntry.offset);
		byteBuffer.get(resourceFileBytes);
		this.resourceFile = new ResourceFile(resourceFileBytes);
	}

	ResourceFile getResourceFile() {
		return resourceFile;
	}

	private record Entry(int entryID, int offset, int length) {
	}
}
