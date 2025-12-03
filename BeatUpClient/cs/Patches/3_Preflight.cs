static partial class BeatUpClient {
	public static async System.Threading.Tasks.Task<AuthenticationToken> AuthWrapper(System.Threading.Tasks.Task<AuthenticationToken> task, AuthenticationToken.PlatformType platform, string userId, string userName) {
		try {
			return await task;
		} catch(System.Security.Authentication.AuthenticationException) {
			return new AuthenticationToken(platform, userId, userName, ""); // fix for offline mode
		}
	}

	[Detour(typeof(PlatformAuthenticationTokenProvider), nameof(PlatformAuthenticationTokenProvider.GetAuthenticationToken))]
	static System.Threading.Tasks.Task<AuthenticationToken> PlatformAuthenticationTokenProvider_GetAuthenticationToken(PlatformAuthenticationTokenProvider self) {
		System.Threading.Tasks.Task<AuthenticationToken> result = (System.Threading.Tasks.Task<AuthenticationToken>)Base(self);
		if(Resolve<CustomNetworkConfig>() != null)
			result = AuthWrapper(result, self._platformType, self._userId, self._userName);
		return result;
	}
}
