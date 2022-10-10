#include <iostream>
#include <sndfile.hh>
#include <vector>
#include <cmath>
using namespace std;

constexpr size_t FRAMES_BUFFER_SIZE = 65536; // Buffer for reading/writing frames

int main(int argc, char *argv[]) {

    if(argc < 3) {
		cerr << "Usage: wav_cp <orignal filename> <compressed filename>" << endl;
		return 1;
	}

    SndfileHandle sfhInOG { argv[argc-2] };
    SndfileHandle sfhInCO { argv[argc-1] };
	if(sfhInOG.error() || sfhInCO.error()) {
		cerr << "Error: invalid input file\n";
		return 1;
    }

	if((sfhInOG.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV || (sfhInCO.format() & SF_FORMAT_TYPEMASK) != SF_FORMAT_WAV) {
		cerr << "Error: file is not in WAV format\n";
		return 1;
	}

	if((sfhInOG.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16 || (sfhInCO.format() & SF_FORMAT_SUBMASK) != SF_FORMAT_PCM_16) {
		cerr << "Error: file is not in PCM_16 format\n";
		return 1;
	}

    //files must have same number of samples
    if((sfhInOG.samplerate() * sfhInOG.channels()) != (sfhInCO.samplerate() * sfhInCO.channels())){
        cerr << "files do not have the same number of samples" << endl;
        return 1;
    }
    
    double N = sfhInOG.samplerate() * sfhInOG.channels();


    double D = 0, S=0;
    size_t nFrames;
    vector<short> OGsamples(FRAMES_BUFFER_SIZE * sfhInOG.channels());
    while((nFrames = sfhInOG.readf(OGsamples.data(), FRAMES_BUFFER_SIZE))) {
        OGsamples.resize(nFrames * sfhInOG.channels());
    }
    // VARS 
    double Ex = 0;
    double Er = 0;
	double SNR = 0;
    vector<short> C0samples(FRAMES_BUFFER_SIZE * sfhInCO.channels());
    while((nFrames = sfhInCO.readf(C0samples.data(), FRAMES_BUFFER_SIZE))) {
        C0samples.resize(nFrames * sfhInCO.channels());
        for(int j=0;j<N; j++){
            if(j%2==0){
                Ex = Ex + OGsamples[j]*OGsamples[j];
                Er = Er + C0samples[j]*C0samples[j];
            }
        }
        SNR = 10 * log10((double)(Ex/Er));

        
    }
	
	
    
}
