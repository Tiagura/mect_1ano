#ifndef WAVQUANT_H
#define WAVQUANT_H

#include <iostream>
#include <vector>

class WAVQuant {
    private:
        short bit_cut;
        size_t n;
        std::vector<short> quant;

    public:
        WAVQuant( size_t n, short bit_cut){
            this->bit_cut = bit_cut; 
            this->n = n;
            quant.resize(n);
        }

        void uniform_scalar(const std::vector<short>& samples) {
            size_t i { };
            //copy samples to quant
            for (auto& s : samples) { 
                quant[i++] = s >> bit_cut;
                //std::cout << s << " " << quant.back() << std::endl;
            }

        }

        //function to print quant
        void print_quant(){
            for (auto& s : quant) { 
                std::cout << s << std::endl;
            }
        }

        //return new vector
        std::vector<short> get_quant()  {
            //copy quant to new vector
            std::vector<short> new_quant;
            new_quant.resize(n);

            size_t i { };
            for (auto& s : quant) { 
                new_quant[i++] = s << bit_cut;
            }
            
            for(size_t i = 0; i < n; i++) {
                std::cout << quant[i] << " " << new_quant[i] << std::endl;
                if(i==10) break;
            }
            return new_quant;
        }

};

#endif 