package au.edu.uq.cai.TissueStack.utils;

import java.util.StringTokenizer;

public final class StringUtils {

		public static final String[] convertCommaSeparatedQueryParamsIntoStringArray (String commaSeparatedQueryParams, boolean eliminateAllWhiteSpace) {
			if (commaSeparatedQueryParams == null) {
				return null;
			}
			if (eliminateAllWhiteSpace) {
				// get rid of whitespace
				commaSeparatedQueryParams = commaSeparatedQueryParams.replaceAll(" ", "");
			}
			if (commaSeparatedQueryParams.isEmpty()) {
				return null;
			}

			final StringTokenizer tokenizer = new StringTokenizer(commaSeparatedQueryParams,",", false);
			final String[] params = new String[tokenizer.countTokens()];
			int i=0;
			while (tokenizer.hasMoreTokens()) {
				params[i] = tokenizer.nextToken();
				i++;
			}
			
			return params;
		}

		public static final Long[] convertCommaSeparatedQueryParamsIntoLongArray (String commaSeparatedQueryParams, boolean eliminateAllWhiteSpace) {
			if (commaSeparatedQueryParams == null) {
				return null;
			}
			if (eliminateAllWhiteSpace) {
				// get rid of whitespace
				commaSeparatedQueryParams = commaSeparatedQueryParams.replaceAll(" ", "");
			}
			if (commaSeparatedQueryParams.isEmpty()) {
				return null;
			}

			final StringTokenizer tokenizer = new StringTokenizer(commaSeparatedQueryParams,",", false);
			final Long[] params = new Long[tokenizer.countTokens()];
			int i=0;
			while (tokenizer.hasMoreTokens()) {
				try {
					params[i] = Long.parseLong(tokenizer.nextToken());
				} catch(NumberFormatException noNumber) {
					throw new IllegalArgumentException("Encounters non-numeric value");
				}
				i++;
			}
			
			return params;
		}
		
		public static final String convertStringArrayIntoCommaSeparatedString(String array[]) {
			if (array == null) {
				return null;
			}
			final StringBuilder stringBuilder = new StringBuilder(array.length * 15);
			for (int i=0; i<array.length;i++) {
				final String value = array[i]; 
				if (value == null || value.trim().isEmpty()) {
					continue;
				}
				stringBuilder.append(",");
				stringBuilder.append(value);
			}
			
			final String commaSeparatedString = stringBuilder.toString();
			if (commaSeparatedString.isEmpty()) {
				return null;
			}
			
			return commaSeparatedString.substring(1);
		}

		public static final String[] prefixStringsInStringArray(String array[], String prefix) {
			if (array == null) {
				return null;
			}
			if (prefix == null || prefix.isEmpty()) {
				return array;
			}
			
			for (int i=0; i<array.length;i++) {
				final String value = array[i]; 
				if (value != null) {
					array[i] = prefix + value; 
				}
			}
			return array;
		}
}