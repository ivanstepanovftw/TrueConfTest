/// 1.заполнить случайными числами от 1 до 9 значения контейнеров vector[i] и map[i];
/// 2.удалить случайное число элементов (не более 15) в каждом контейнере;
/// 3.после этого провести синхронизацию, чтобы в vector и map остались только имеющиеся в обоих контейнерах элементы (дубликаты не удалять).
// Example output:
// /home/user/github.com/ivanstepanovftw/TrueConfTest/cmake-build-debug/TrueConfTest
// vector fill: done in 0.0147962 seconds
// map fill: done in 3.847e-06 seconds
// remove from vector: 2
// remove from map: 2
// vector: 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
// map: [0,1],[1,1],[2,3],[3,5],[4,1],[5,2],[6,4],[7,7],[8,5],[9,8],[10,8],[11,4],[12,6],[13,3],[14,6],[15,7],[16,7],[17,6]
// diff:
// 1: 15
// 2: -1
// 3: -2
// 4: -2
// 5: -2
// 6: -3
// 7: -3
// 8: -2
// 9: 0
// diff:
// 1: 0
// 2: 0
// 3: 0
// 4: 0
// 5: 0
// 6: 0
// 7: 0
// 8: 0
// 9: 0
// sync_remove: done in 0.000191135 seconds
// sync: done in 0.000403236 seconds
// vector: 1,1,1
// map: [0,1],[1,1],[4,1]
//
// Process finished with exit code 0

#include <iostream>
#include <vector>
#include <map>
#include <random>
#include <chrono>
#include <functional>

class Timer {
public:
    explicit Timer(std::string what)
            : m_what(std::move(what)), m_tp(std::chrono::high_resolution_clock::now()) {}

    ~Timer() {
        std::cout << m_what << ": done in " << std::chrono::duration_cast<std::chrono::duration<double>>(
                std::chrono::high_resolution_clock::now() - m_tp).count() << " seconds" << std::endl;
    }

private:
    std::string m_what;
    std::chrono::high_resolution_clock::time_point m_tp;
};



int main() {
    using std::cout, std::endl;

    using number = uint8_t;
    // constexpr size_t m = 1000000000; // elements count
    // constexpr size_t m = 100; // elements count
    constexpr size_t m = 20; // elements count
    std::uniform_int_distribution<number> dis_gen(1, 9); // inclusive
    std::uniform_int_distribution<size_t> dis_rm(1, 15); // how much elements to remove from container


    std::mt19937_64 r(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    auto gen = std::bind(dis_gen, r);

    std::vector<number> vector;
    std::map<size_t, number> map;

    {
        Timer timer{"vector fill"};
        // std::generate(map.begin(), map.end(), gen);
        vector.resize(m);
        #pragma omp parallel for firstprivate(gen)
        for (size_t i=0; i<m; i++) {
            vector[i] = gen();
        }
    }
    {
        Timer timer{"map fill"};
        for (size_t i=0; i<m; i++) {
            map[i] = gen();
        }
    }


    size_t r_vector = std::min(dis_rm(r), vector.size());
    size_t r_map = std::min(dis_rm(r), map.size());

    cout<<"remove from vector: "<<r_vector<<endl;
    vector.resize(vector.size() - r_vector);

    cout<<"remove from map: "<<r_map<<endl;
    auto mf = map.end();
    std::advance(mf, -r_map);
    map.erase(mf, map.end());


    cout<<"vector: ";
    for (auto& a : vector) {
        cout<<+a<<',';
    }
    cout<<'\b'<<endl;

    cout<<"map: ";
    for (auto& a : map) {
        cout<<"["<<a.first<<","<<+a.second<<"],";
    }
    cout<<'\b'<<endl;

    {
        Timer timer_sync{"sync"};
        std::map<number, ssize_t> diff_numbers; // diff_numbers = vector, tmp = map, diff_numbers -= tmp
        std::map<number, ssize_t> tmp;  // ssize_t{} == 0, поэтому не обнуляем

        for (size_t i=0; i<vector.size(); i++) {
            diff_numbers[vector[i]]++;
        }
        for (const auto& e : map) {
            tmp[e.second]++;
        }
        for (number c = dis_gen.min(); c <= dis_gen.max(); c++) {
            diff_numbers[c] -= tmp[c];
        }

        // print difference
        cout<<"diff: "<<endl;
        for(auto& a : diff_numbers) {
            cout<<+a.first<<": "<<a.second<<endl;
        }


        Timer timer_sync_remove{"sync_remove"};
        vector.erase(std::remove_if(vector.begin(), vector.end(), [&](const number& n)->bool{
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

        // print difference
        cout<<"diff: "<<endl;
        for(auto& a : diff_numbers) {
            cout<<+a.first<<": "<<a.second<<endl;
        }
    }

    cout<<"vector: ";
    for (auto& a : vector) {
        cout<<+a<<',';
    }
    cout<<'\b'<<endl;

    cout<<"map: ";
    for (auto& a : map) {
        cout<<"["<<a.first<<","<<+a.second<<"],";
    }
    cout<<'\b'<<endl;
}
