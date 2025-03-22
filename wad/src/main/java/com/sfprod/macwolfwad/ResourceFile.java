package com.sfprod.macwolfwad;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import com.sfprod.utils.NumberUtils;

/**
 * @see <a href=
 *      "https://github.com/kreativekorp/ksfl/wiki/Macintosh-Resource-File-Format">Macintosh
 *      Resource File Format</a>
 */
class ResourceFile {

	private final List<Type> types;

	ResourceFile(String filename) {
		byte[] bytes = MacWolfWadFactory.getBytes(filename);
		ByteBuffer byteBuffer = ByteBuffer.wrap(bytes);
		int resourceDataOffset = byteBuffer.getInt();
		int resourceMapOffset = byteBuffer.getInt();
		int resourceDataSize = byteBuffer.getInt();
		int resourceMapSize = byteBuffer.getInt();

		byteBuffer.position(resourceMapOffset);

		int resourceDataOffsetCopy = byteBuffer.getInt();
		int resourceMapOffsetCopy = byteBuffer.getInt();
		int resourceDataSizeCopy = byteBuffer.getInt();
		int resourceMapSizeCopy = byteBuffer.getInt();

		assert resourceDataOffset == resourceDataOffsetCopy;
		assert resourceMapOffset == resourceMapOffsetCopy;
		assert resourceDataSize == resourceDataSizeCopy;
		assert resourceMapSize == resourceMapSizeCopy;

		int nextResourceMap = byteBuffer.getInt(); // used by the Resource Manager for internal bookkeeping
		short fileRef = byteBuffer.getShort(); // used by the Resource Manager for internal bookkeeping
		short attributes = byteBuffer.getShort();
		short typeListOffset = byteBuffer.getShort();
		short nameListOffset = byteBuffer.getShort();

		byteBuffer.position(resourceMapOffset + typeListOffset);

		short typeCount = byteBuffer.getShort();

		this.types = new ArrayList<>(typeCount + 1);
		for (int i = 0; i <= typeCount; i++) {
			byte[] type = new byte[4];
			byteBuffer.get(type);
			short resourceCount = byteBuffer.getShort();
			short resourceListOffset = byteBuffer.getShort();
			types.add(
					new Type(new String(type), resourceCount, resourceListOffset, new ArrayList<>(resourceCount + 1)));
		}

		for (Type type : types) {
			byteBuffer.position(resourceMapOffset + typeListOffset + type.resourceListOffset());
			for (int i = 0; i <= type.resourceCount(); i++) {
				short id = byteBuffer.getShort();
				short nameOffset = byteBuffer.getShort();
				int attributesAndDataOffset = byteBuffer.getInt();
				int resourcePtr = byteBuffer.getInt(); // used by the Resource Manager for internal bookkeeping
				type.addResource(
						new Resource(id, nameOffset, NumberUtils.toByte((attributesAndDataOffset & 0xFF000000) >> 24),
								attributesAndDataOffset & 0x00FFFFFF));
			}

			for (Resource resource : type.resourceList()) {
				// name
				if (resource.nameOffset() != -1) {
					byteBuffer.position(resourceMapOffset + nameListOffset + resource.nameOffset());
					byte length = byteBuffer.get();
					byte[] name = new byte[length];
					byteBuffer.get(name);
					resource.setName(name);
				}

				// data
				byteBuffer.position(resourceDataOffset + resource.dataOffset());
				int size = byteBuffer.getInt();
				byte[] data = new byte[size];
				byteBuffer.get(data);
				resource.setData(data);
			}
		}
	}

	Type getType(String resourceType) {
		return types.stream().filter(t -> Objects.equals(resourceType, t.type())).findAny().orElseThrow();
	}

}
