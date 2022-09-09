static partial class BeatUpClient {
	public static async System.Threading.Tasks.Task<AuthenticationToken> AuthWrapper(System.Threading.Tasks.Task<AuthenticationToken> task, AuthenticationToken.Platform platform, string userId, string userName) {
		try {
			return await task;
		} catch(System.Security.Authentication.AuthenticationException) {
			return new AuthenticationToken(platform, userId, userName, ""); // fix for offline mode
		}
	}

	[Patch(PatchType.Postfix, typeof(PlatformAuthenticationTokenProvider), nameof(PlatformAuthenticationTokenProvider.GetAuthenticationToken))]
	public static void PlatformAuthenticationTokenProvider_GetAuthenticationToken(ref System.Threading.Tasks.Task<AuthenticationToken> __result, AuthenticationToken.Platform ____platform, string ____userId, string ____userName) {
		if(Resolve<CustomNetworkConfig>() != null)
			__result = AuthWrapper(__result, ____platform, ____userId, ____userName);
	}
}
