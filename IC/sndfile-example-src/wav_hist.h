#ifndef WAVHIST_H
#define WAVHIST_H

#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <sndfile.hh>

class WAVHist {
  private:
	std::vector<std::map<short, size_t>> counts;


  public:
	WAVHist(const SndfileHandle& sfh) {
		counts.resize(sfh.channels());
	}

	void update(const std::vector<short>& samples) {
		size_t n { };
		for(auto s : samples)
		 	counts[n++ % counts.size()][s]++;
		
		std::map<short, size_t> MID;
		std::map<short, size_t> SIDE;
		short aux { 0 };
		n=0;
		for(auto s : samples){
			if(n%2 == 0){
				aux = s;
			}else{	
				//print aux and s
				//std::cout << aux << " " << s << " MID:" <<(aux+s)/2 << " SIDE: "<< (aux-s)/2 << std::endl;
				MID[(aux+s)/2]++;
				SIDE[(aux-s)/2]++;
			}
			n++;
		}

	    //MID CHANNEL
		std::ofstream out_file("MID_channel.txt");
		for(auto [value, counter] : MID)
			out_file << value << '\t' << counter << '\n';
		out_file.close();

		//SIDE CHANNEL
		std::ofstream out_file2("SIDE_channel.txt");
		for(auto [value, counter] : SIDE)
			out_file2 << value << '\t' << counter << '\n';
	}

	void dump(const size_t channel) const {
		for(auto [value, counter] : counts[channel])
			std::cout << value << '\t' << counter << '\n';
	}

	// void MID_channel() {
	// 	std::map<short, size_t> hist { counts[0] };
	// 	for(auto [value, counter] : counts[1])
	// 		hist[value] += counter;

	// 	std::ofstream out_file("MID_channel.txt");
	// 	for(auto [value, counter] : hist)
	// 		out_file << value << '\t' << counter << '\n';

	// 	out_file.close();
		
	// }

	// void SIDE_channel() {
	// 	std::map<short, size_t> hist { counts[0] };
	// 	for(auto [value, counter] : counts[1])
	// 		hist[value] -= counter;

	// 	std::ofstream out_file("SIDE_channel.txt");
	// 	for(auto [value, counter] : hist)
	// 		out_file << value << '\t' << counter << '\n';

	// 	out_file.close();
		
	// }
};

#endif 

