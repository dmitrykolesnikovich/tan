#include "cli.h"
#include "base.h"
#include <gtest/gtest.h>

#ifndef TAN_PROJECT_SOURCE_DIR
#error "Define TAN_PROJECT_SOURCE_DIR before compiling this"
#endif

class MyFixture : public ::testing::Test {};

class TanCTests : public MyFixture {
public:
  TanCTests(const str &filename) : _filename(filename) {}
  void TestBody() override {
    vector<const char *> cmd
        {"tanc", "--print-ast", "--print-ir", "-I" __STR__(TAN_PROJECT_SOURCE_DIR), _filename.c_str(), "-lruntime",
            "-L" __STR__(TAN_PROJECT_SOURCE_DIR) "/runtime", "-o", "a.out"};
    int argc = static_cast<int>(cmd.size());
    auto *argv = c_cast(char**, cmd.data());
    EXPECT_EQ(0, cli_main(argc, argv));
    EXPECT_EQ(0, system("./a.out"));
  }

private:
  str _filename;
};

void register_tanc_test(const str &v) {
  ::testing::RegisterTest("tanc_test_fixture",
      ("test_" + v).c_str(),
      nullptr,
      v.c_str(),
      __FILE__,
      __LINE__,
      [=]() -> MyFixture * { return new TanCTests(v); });
}

int main(int argc, char **argv) {
  assert(argc == 2);
  ::testing::InitGoogleTest(&argc, argv);
  register_tanc_test(argv[1]);
  return RUN_ALL_TESTS();
}
