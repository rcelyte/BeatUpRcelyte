static unsafe class BeatUpClient_Beta {
	[System.Runtime.InteropServices.StructLayout(System.Runtime.InteropServices.LayoutKind.Sequential, CharSet = System.Runtime.InteropServices.CharSet.Unicode)]
	struct TxtRecord {
		public TxtRecord *pNext;
		public System.IntPtr pName;
		public short wType, wDataLength;
		public uint flags, dwTtl, dwReserved, dwStringCount;
		public System.IntPtr pStringArray;
	}
	[System.Runtime.InteropServices.DllImport("Dnsapi.dll", EntryPoint = "DnsQuery_W", ExactSpelling = true, CharSet = System.Runtime.InteropServices.CharSet.Unicode, SetLastError = true)]
	static extern int DnsQuery(string lpstrName, short wType, int options, System.IntPtr pExtra, ref TxtRecord* ppQueryResultsSet, System.IntPtr pReserved);
	[System.Runtime.InteropServices.DllImport("Dnsapi.dll")]
	static extern void DnsRecordListFree(TxtRecord* pRecordList, int freeType);
	static System.Collections.Generic.List<string> GetTxtRecords(string domain) {
		TxtRecord *resultSet = null;
		System.Collections.Generic.List<string> result = new System.Collections.Generic.List<string>();
		try {
			if(DnsQuery(domain, 0x0010, 0x00000000, System.IntPtr.Zero, ref resultSet, System.IntPtr.Zero) != 0)
				return result;
			for(TxtRecord *record = resultSet; record != null; record = record->pNext) {
				if(record->wType != 0x0010)
					continue;
				System.Text.StringBuilder builder = new System.Text.StringBuilder();
				System.IntPtr* stringArray = &record->pStringArray;
				for(uint i = 0; i < record->dwStringCount; ++i)
					builder.Append(System.Runtime.InteropServices.Marshal.PtrToStringUni(stringArray[i]));
				result.Add(builder.ToString());
			}
		} finally {
			if(resultSet != null)
				DnsRecordListFree(resultSet, 1);
		}
		return result;
	}
	public static string? CheckVersion(Hive.Versioning.Version version) {
		foreach(string record in GetTxtRecords("beatup_beta.battletrains.org")) {
			string[] parts = record.Split(new[] {'|'}, 2);
			if(parts.Length != 2)
				continue;
			foreach(string range in parts[0].Split(new[] {';'}))
				if((new Hive.Versioning.VersionRange(range)).Matches(version))
					return null;
			return parts[1];
		}
		return string.Empty;
	}
}
