# TODO
Questions:
    - what is pssh? why do we use it?
    - manifest.mpd contents. understand each element
    - manifest.m3u8 contents. understand each element
    - basic flow
        - load manifest
        - extract keyids
        - acquire license
        - setup pipeline with decryptor
        - 
    - differences in license servers (widevine and playready)

libgstvaldrmort is a decryptor element.
    - It should be able to do CENC and CBCS (support for decryption using openssl)

inside transform_ip we can get gst_buffer_get_protection_meta
from prot_meta we can get kid and iv


App should load the configuration
GST plugin should have parameters we can set from the app?


how do we feed kid and key pairs to the decryptor plugin from the app?
- a file
- via parameters
- some other method?


A class to manage configuration options/parameters
- load default configuration from "/etc/valdrmort.conf"
- user can provide custom configuration via command line
- user can provide options via command line

# configuration
borrow ideas from shakaplayer configuration
- manifestURI
- licenseServer
- drm
    - servers
    - headers e.g. add "X-AxDRM-Message"
    - clearkeys
    - key systems by URI


const manifestUri =
    'https://storage.googleapis.com/shaka-demo-assets/sintel-widevine/dash.mpd';
const licenseServer = 'https://cwip-shaka-proxy.appspot.com/no_auth';

  drm: {
    servers: {
      'com.widevine.alpha': 'https://foo.bar/drm/widevine',
      'com.microsoft.playready': 'https://foo.bar/drm/playready',
            'org.w3.clearkey': 'http://foo.bar/drm/clearkey'
    }
  }


    manifest: {
    dash: {
      keySystemsByURI: {
        'urn:uuid:9a04f079-9840-4286-ab92-e65be0885f95': 'com.microsoft.playready.recommendation',
        'urn:uuid:79f0049a-4098-8642-ab92-e65be0885f95': 'com.microsoft.playready.recommendation',
      }
  }

  drm: {
    clearKeys: {
      // 'key-id-in-hex': 'key-in-hex',
      'deadbeefdeadbeefdeadbeefdeadbeef': '18675309186753091867530918675309',
      '02030507011013017019023029031037': '03050701302303204201080425098033'
    }
  }

# Flows
- how does the plugin work internally
- how does the gst pipeline work
- how does the application work
    - configuration
    - manifest parsing
    - license acquisition
    - pipeline creation
    - playback

# Test
- ManifestParsers - should handle given mpd or m3u8 file, extract key ids
- Encryption/Decryption - 
- LicenseRequest - use Axinom's server to acquire a license
- 

