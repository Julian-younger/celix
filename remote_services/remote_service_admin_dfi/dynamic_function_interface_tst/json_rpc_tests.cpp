/**
 * Licensed under Apache License v2. See LICENSE for more information.
 */
#include <CppUTest/TestHarness.h>
#include <float.h>
#include <assert.h>
#include "CppUTest/CommandLineTestRunner.h"                                                                                                                                                                        

extern "C" {
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <ffi.h>

#include "dyn_common.h"
#include "dyn_type.h"
#include "json_serializer.h"
#include "json_rpc.h"

static void stdLog(void *handle, int level, const char *file, int line, const char *msg, ...) {
    va_list ap;
    const char *levels[5] = {"NIL", "ERROR", "WARNING", "INFO", "DEBUG"};
    fprintf(stderr, "%s: FILE:%s, LINE:%i, MSG:",levels[level], file, line);
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    fprintf(stderr, "\n");
}


    void prepareTest(void) {
        dyn_function_type *dynFunc = NULL;
        int rc = dynFunction_parseWithStr("add(#am=handle;PDD#am=pre;*D)N", NULL, &dynFunc);
        CHECK_EQUAL(0, rc);

        char *result = NULL;

        void *handle = NULL;
        double arg1 = 1.0;
        double arg2 = 2.0;

        void *args[4];
        args[0] = &handle;
        args[1] = &arg1;
        args[2] = &arg2;

        rc = jsonRpc_prepareInvokeRequest(dynFunc, "add", args, &result);
        CHECK_EQUAL(0, rc);

        //printf("result is %s\n", result);

        STRCMP_CONTAINS("\"add\"", result);
        STRCMP_CONTAINS("1.0", result);
        STRCMP_CONTAINS("2.0", result);

        free(result);
        dynFunction_destroy(dynFunc);
    }

    void handleTest(void) {
        dyn_function_type *dynFunc = NULL;
        int rc = dynFunction_parseWithStr("add(#am=handle;PDD#am=pre;*D)N", NULL, &dynFunc);
        CHECK_EQUAL(0, rc);

        const char *reply = "{\"r\":2.2}";
        double result = -1.0;
        double *out = &result;
        void *args[4];
        args[3] = &out;
        rc = jsonRpc_handleReply(dynFunc, reply, args);
        CHECK_EQUAL(0, rc);
        //CHECK_EQUAL(2.2, result);

        dynFunction_destroy(dynFunc);
    }

    int add(void *handle, double a, double b, double *result) {
        *result = a + b;
        return 0;
    }

    struct tst_seq {
        uint32_t cap;
        uint32_t len;
        double *buf;
    };


    //StatsResult={DDD[D average min max input}
    struct tst_StatsResult {
        double average;
        double min;
        double max;
        struct tst_seq input;
    };


    int stats(void *handle, struct tst_seq input, struct tst_StatsResult **out) {
        assert(out != NULL);
        assert(*out == NULL);
        double total = 0.0;
        int count = 0;
        double max = DBL_MIN;
        double min = DBL_MAX;

        int i;
        for (i = 0; i<input.len; i += 1) {
            total += input.buf[i];
            count += 1;
            if (input.buf[i] > max) {
                max = input.buf[i];
            }
            if (input.buf[i] < min) {
                min = input.buf[i];
            }
        }

        struct tst_StatsResult *result = (struct tst_StatsResult *) calloc(1, sizeof(*result));
        result->average = total / count;
        result->min = min;
        result->max = max;
        double *buf = (double *)calloc(input.len, sizeof(double));
        memcpy(buf, input.buf, input.len * sizeof(double));
        result->input.len = input.len;
        result->input.cap = input.len;
        result->input.buf = buf;

        *out = result;
        return 0;
    }

    struct tst_serv {
        void *handle;
        int (*add)(void *, double, double, double *);
        int (*sub)(void *, double, double, double *);
        int (*sqrt)(void *, double, double *);
        int (*stats)(void *, struct tst_seq, struct tst_StatsResult **);
    };

    void callTestPreAllocated(void) {
        dyn_interface_type *intf = NULL;
        FILE *desc = fopen("descriptors/example1.descriptor", "r");
        CHECK(desc != NULL);
        int rc = dynInterface_parse(desc, &intf);
        CHECK_EQUAL(0, rc);

        char *result = NULL;

        struct tst_serv serv;
        serv.handle = NULL;
        serv.add = add;


        rc = jsonRpc_call(intf, &serv, "{\"m\":\"add(DD)D\", \"a\": [1.0,2.0]}", &result);
        CHECK_EQUAL(0, rc);
        STRCMP_CONTAINS("3.0", result);

        free(result);
        dynInterface_destroy(intf);
    }

    void callTestOutput(void) {
        dyn_interface_type *intf = NULL;
        FILE *desc = fopen("descriptors/example1.descriptor", "r");
        CHECK(desc != NULL);
        int rc = dynInterface_parse(desc, &intf);
        CHECK_EQUAL(0, rc);

        char *result = NULL;

        struct tst_serv serv;
        serv.handle = NULL;
        serv.stats = stats;

        rc = jsonRpc_call(intf, &serv, "{\"m\":\"stats([D)LStatsResult;\", \"a\": [[1.0,2.0]]}", &result);
        CHECK_EQUAL(0, rc);
        STRCMP_CONTAINS("1.5", result); //avg

        free(result);
        dynInterface_destroy(intf);
    }

}

TEST_GROUP(JsonRpcTests) {
    void setup() {
        int lvl = 4;
        dynCommon_logSetup(stdLog, NULL, lvl);
        dynType_logSetup(stdLog, NULL,lvl);
        dynFunction_logSetup(stdLog, NULL,lvl);
        dynInterface_logSetup(stdLog, NULL,lvl);
        jsonSerializer_logSetup(stdLog, NULL, lvl);
        jsonRpc_logSetup(stdLog, NULL, lvl);

    }
};


TEST(JsonRpcTests, prepareTest) {
    prepareTest();
}

TEST(JsonRpcTests, handleTest) {
    handleTest();
}

TEST(JsonRpcTests, callPre) {
    callTestPreAllocated();
}

TEST(JsonRpcTests, callOut) {
    callTestOutput();
}


