// Manifest dependency for common controls.
//
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

extern "C"
{
	// Libtommath random-source stubs.
	//
	// The library probes for a small set of platform RNG providers. On Windows
	// these entry points do not exist, but the symbols must still be present for
	// the build to succeed (meh).
	//
	int s_read_arc4random (void*, std::size_t) {return -1;}
	int s_read_getrandom  (void*, std::size_t) {return -1;}
	int s_read_urandom    (void*, std::size_t) {return -1;}
	int s_read_ltm_rng    (void*, std::size_t) {return -1;}
};
