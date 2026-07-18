static partial class BeatUpClient {
	public static async System.Threading.Tasks.Task<XPlatformAccessTokenData> AuthWrapper(
			System.Threading.Tasks.Task<XPlatformAccessTokenData> task, AuthenticationToken.PlatformType platform, string userId, string userName) {
		try {
			return await task;
		} catch(System.TimeoutException) {
			return new XPlatformAccessTokenData(new string('#', XPlatformAccessTokenData.kMinimalTokenLength), PlatformEnvironment.Production); // fix for offline mode
		}
	}

	[Detour(typeof(PlatformAuthenticationTokenProvider), nameof(PlatformAuthenticationTokenProvider.GetXPlatformAccessToken))]
	static System.Threading.Tasks.Task<XPlatformAccessTokenData> PlatformAuthenticationTokenProvider_GetXPlatformAccessToken(
			PlatformAuthenticationTokenProvider self, System.Threading.CancellationToken cancellationToken, bool skipCacheRead) {
		System.Threading.Tasks.Task<XPlatformAccessTokenData> result = (System.Threading.Tasks.Task<XPlatformAccessTokenData>)Base(self, cancellationToken, skipCacheRead);
		if(Resolve<CustomNetworkConfig>() != null)
			result = AuthWrapper(result, self._platformType, self._userId, self._userName);
		return result;
	}
}
