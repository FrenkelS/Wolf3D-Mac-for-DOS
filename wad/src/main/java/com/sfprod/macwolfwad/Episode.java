package com.sfprod.macwolfwad;

enum Episode {
	FIRST_ENCOUNTER("Wolfenstein 3D™ First Encounter", "MACWOLF1.WAD"), //
	SECOND_ENCOUNTER("Wolfenstein 3D™", "MACWOLF2.WAD");

	private final String inputFilename;
	private final String outputFilename;

	Episode(String inputFilename, String outputFilename) {
		this.inputFilename = inputFilename;
		this.outputFilename = outputFilename;
	}

	String getInputFilename() {
		return inputFilename;
	}

	String getOutputFilename() {
		return outputFilename;
	}
}
