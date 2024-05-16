/*********************************************************************
 * @file    template1_bench.cpp
 * @brief
 * @details
 * @author  Shivelino
 * @date    2024.5.16
 *********************************************************************/
#include <benchmark/benchmark.h>

// ACIS
#include <acis/cstrapi.hxx>
#include <acis_utils/acis_utils.hpp>
#include <acis_utils/same_entity.hpp>  // same_entity所在头文件

// GME
#include "template/template1.hxx"

class Template1_Sample : public benchmark::Fixture {
    int level = 0;

  protected:
    void SetUp(benchmark::State& state) override { level = initialize_acis(); }

    void TearDown(benchmark::State& state) override { terminate_acis(level); }
};

BENCHMARK_DEFINE_F(Template1_Sample, acis_api_make_cuboid)(benchmark::State& state) {
    BODY* en = nullptr;
    for(auto _: state) {
        api_make_cuboid(1.0, 2.0, 3.0, en);
    }
}

BENCHMARK_REGISTER_F(Template1_Sample, acis_api_make_cuboid)->Unit(benchmark::kMillisecond);

BENCHMARK_DEFINE_F(Template1_Sample, gme_api_make_cuboid)(benchmark::State& state) {
    BODY* en = nullptr;
    for(auto _: state) {
        gme_api_make_cuboid(1.0, 2.0, 3.0, en);
    }
}

BENCHMARK_REGISTER_F(Template1_Sample, gme_api_make_cuboid)->Unit(benchmark::kMillisecond);
