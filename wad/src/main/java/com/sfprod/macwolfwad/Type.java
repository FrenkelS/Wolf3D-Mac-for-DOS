package com.sfprod.macwolfwad;

import java.util.List;

record Type(String type, short resourceCount, short resourceListOffset, List<Resource> resourceList) {

	void addResource(Resource resource) {
		resourceList.add(resource);
	}

	int calculateMaxId() {
		return resourceList.stream().mapToInt(Resource::id).max().orElseThrow();
	}

}
