// Manifest dependency for common controls. GCC has no such pragma, so with
// MinGW we embed the same dependency with iw4x.manifest (see Resource.rc).
//
#ifdef _MSC_VER
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

extern "C"
{
  // Enable 'High Performance Graphics'.
  //
  // Official documentation states that this mechanism is not supported when
  // invoked from a DLL. Turn out that in practice, user reports and field
  // testing indicate that it does actually take effect and is in fact required
  // for hybrid (Optimus) system. We therefore enable it here despite the
  // documented limitation.
  //
  // https://docs.nvidia.com/gameworks/content/technologies/desktop/optimus
  // https://gpuopen.com/learn/amdpowerxpressrequesthighperformance/
  //
  __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
  __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;

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
