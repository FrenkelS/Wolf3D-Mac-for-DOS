package com.sfprod.macwolfwad;

import java.util.Comparator;
import java.util.List;

record Type(String type, short resourceCount, short resourceListOffset, List<Resource> resourceList) {

	void addResource(Resource resource) {
		resourceList.add(resource);
	}

	int calculateMaxId() {
		return resourceList.stream().mapToInt(Resource::id).max().orElseThrow();
	}

	void printResourceList() {
		resourceList.stream().sorted(Comparator.comparing(Resource::id))
				.forEach(r -> System.out.println(r.id() + ": " + new String(r.getName())));
	}

}
