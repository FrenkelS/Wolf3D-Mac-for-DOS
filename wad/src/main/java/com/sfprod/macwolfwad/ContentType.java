package com.sfprod.macwolfwad;

enum ContentType {
	FIRST_ENCOUNTER("1103622D", "MACWOLF1.WAD"), //
	SECOND_ENCOUNTER("40698D57", "MACWOLF2.WAD"), //
	THIRD_ENCOUNTER("FA11B20B", "MACWOLF2.WAD"), //

	SECOND_ENCOUNTER_MAPS("", ""), //

	THIRD_ENCOUNTER_EPISODE1_MAPS("BED89984", "MW3E1.WAD"), //
	THIRD_ENCOUNTER_EPISODE2_MAPS("71710DDA", "MW3E2.WAD"), //
	THIRD_ENCOUNTER_EPISODE3_MAPS("7DE52050", "MW3E3.WAD"), //
	THIRD_ENCOUNTER_EPISODE4_MAPS("754DF1FA", "MW3E4.WAD"), //
	THIRD_ENCOUNTER_EPISODE5_MAPS("E1D5D0E8", "MW3E5.WAD"), //
	THIRD_ENCOUNTER_EPISODE6_MAPS("52284BCE", "MW3E6.WAD"), //

	UNKNOWN_MAPS("", "");

	private final String crc32;
	private final String outputFilename;

	ContentType(String crc32, String outputFilename) {
		this.crc32 = crc32;
		this.outputFilename = outputFilename;
	}

	String getCrc32() {
		return crc32;
	}

	String getOutputFilename() {
		return outputFilename;
	}

	static ContentType determineContentType(int length) {
		return switch (length) {
		case 1_537_987 -> FIRST_ENCOUNTER; // 1.0
		case 1_537_319 -> FIRST_ENCOUNTER; // 1.0.1
		case 2_441_827 -> SECOND_ENCOUNTER;
		case 2_424_697 -> THIRD_ENCOUNTER;

		case 270_327 -> SECOND_ENCOUNTER_MAPS;

		case 119_368 -> THIRD_ENCOUNTER_EPISODE1_MAPS;
		case 113_866 -> THIRD_ENCOUNTER_EPISODE2_MAPS;
		case 99_892 -> THIRD_ENCOUNTER_EPISODE3_MAPS;
		case 119_452 -> THIRD_ENCOUNTER_EPISODE4_MAPS;
		case 90_453 -> THIRD_ENCOUNTER_EPISODE5_MAPS;
		case 132_260 -> THIRD_ENCOUNTER_EPISODE6_MAPS;

		default -> UNKNOWN_MAPS;
		};
	}
}
