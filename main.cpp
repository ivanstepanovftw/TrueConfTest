/// 1.заполнить случайными числами от 1 до 9 значения контейнеров vector[i] и map[i];
/// 2.удалить случайное число элементов (не более 15) в каждом контейнере;
/// 3.после этого провести синхронизацию, чтобы в vector и map остались только имеющиеся в обоих контейнерах элементы (дубликаты не удалять).

#include <iostream>
#include <vector>
#include <map>
#include <random>
#include <chrono>
#include <functional>


int main() {
    using std::cout, std::endl;

    using number = uint8_t;
    constexpr size_t m = 1000000; // elements count
    // constexpr size_t m = 100; // elements count
    // constexpr size_t m = 20; // elements count
    std::uniform_int_distribution<number> dis_gen(1, 9); // inclusive
    std::uniform_int_distribution<size_t> dis_rm(1, 15); // how much elements to remove from container


    std::vector<number> vector;
    std::map<size_t, number> map;

    std::mt19937_64 r(std::chrono::high_resolution_clock::now().time_since_epoch().count());

    {
        // fill vector and map with random numbers in 1..9
        auto gen = std::bind(dis_gen, r);
        vector.resize(m);
        std::generate(vector.begin(), vector.end(), gen);
        for (size_t i=0; i<m; i++) {
            map[i] = gen();
        }
    }

    {
        // remove `r_vector` elements from end of `vector`, remove `r_map` elements from end of `map`
        size_t r_vector = std::min(dis_rm(r), vector.size());
        size_t r_map = std::min(dis_rm(r), map.size());
        vector.resize(vector.size() - r_vector);
        auto mf = map.end();
        std::advance(mf, -r_map);
        map.erase(mf, map.end());
    }

    {
        // count each number in `vector`, then subtract each number from `map` respectively.
        // then remove `diff_numbers[n]` numbers for number `n` from `vector`
        // then remove `-diff_numbers[n]` numbers for number `n` from `map`
        std::map<number, ssize_t> diff_numbers;

        for (const auto& i : vector) {
            diff_numbers[i]++;
        }
        for (const auto& e : map) {
            diff_numbers[e.second]--;
        }

        vector.erase(std::remove_if(vector.begin(), vector.end(), [&](const number& n) {
                         if (diff_numbers[n] > 0) {
                             diff_numbers[n]--;
                             return true;
                         }
                         return false;
                     }),
                     vector.end());

        auto predicate = [&](const auto& kv)->bool{
            if (diff_numbers[kv.second] < 0) {
                diff_numbers[kv.second]++;
                return true;
            }
            return false;
        };
        for (auto it = map.begin(); it != map.end();) {
            if (predicate(*it))
                it = map.erase(it);
            else
                ++it;
        }

        // print difference, must be filled with zero
        cout<<"diff (must be filled with zero): "<<endl;
        for(auto& a : diff_numbers) {
            cout<<+a.first<<": "<<a.second<<endl;
        }
    }
}
