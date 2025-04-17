package com.sfprod.macwolfwad;

enum Episode {
	FIRST_ENCOUNTER("Wolfenstein 3D™ First Encounter", "AB56140B", "MACWOLF1.WAD"), //
	SECOND_ENCOUNTER("Wolfenstein 3D™", "59C56E62", "MACWOLF2.WAD"), //
	THIRD_ENCOUNTER("Wolfenstein 3D™", "C004B007", "MACWOLF2.WAD");

	static final String SECOND_ENCOUNTER_SEPARATE_MAPS_FILENAME = "Second Encounter (30 Levels)";

	private final String inputFilename;
	private final String crc;
	private final String outputFilename;

	Episode(String inputFilename, String crc, String outputFilename) {
		this.inputFilename = inputFilename;
		this.crc = crc;
		this.outputFilename = outputFilename;
	}

	String getInputFilename() {
		return inputFilename;
	}

	String getCrc() {
		return crc;
	}

	String getOutputFilename() {
		return outputFilename;
	}
}
