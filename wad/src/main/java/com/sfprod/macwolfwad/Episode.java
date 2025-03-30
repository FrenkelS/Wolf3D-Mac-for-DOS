package com.sfprod.macwolfwad;

enum Episode {
	FIRST_ENCOUNTER("Wolfenstein 3D™ First Encounter", null, "MACWOLF1.WAD"), //
	SECOND_ENCOUNTER("Wolfenstein 3D™", "Second Encounter (30 Levels)", "MACWOLF2.WAD");

	private final String inputFilename;
	private final String mapInputFilename;
	private final String outputFilename;

	Episode(String inputFilename, String mapInputFilename, String outputFilename) {
		this.inputFilename = inputFilename;
		this.mapInputFilename = mapInputFilename;
		this.outputFilename = outputFilename;
	}

	String getInputFilename() {
		return inputFilename;
	}

	String getMapInputFilename() {
		return mapInputFilename;
	}

	String getOutputFilename() {
		return outputFilename;
	}
}
