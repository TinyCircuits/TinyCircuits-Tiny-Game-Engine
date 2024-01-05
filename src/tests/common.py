def run_tests(test_list):
    result_list = []

    for test in test_list:
        test_name = test[0]
        test_func = test[1]
        print("RUNNING TEST: " + test_name)
        result_list.append([test_name, test_func()])
    
    fail_count = 0

    for result in result_list:
        test_name = result[0]
        test_passed = result[1]
        print("TEST: " + test_name)
        if test_passed:
            print("\tPASSED")
        else:
            fail_count = fail_count + 1
            print("\tFAILED")
    
    print("\nFAIL COUNT: " + str(fail_count))