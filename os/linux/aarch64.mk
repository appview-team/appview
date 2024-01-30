ARCH_CFLAGS=-D__GO__ -ffixed-x28 
ARCH_LD_FLAGS=-lcapstone
ARCH_BINARY=elf64-littleaarch64
ARCH_OBJ=$(ARCH)

LD_FLAGS=$(MUSL_AR) $(UNWIND_AR) $(COREDUMPER_AR) $(PCRE2_AR) $(LS_HPACK_AR) $(YAML_AR) $(JSON_AR) -ldl -lpthread -lrt -lresolv -lz -Lcontrib/build/funchook -lfunchook -Lcontrib/build/funchook/capstone_src-prefix/src/capstone_src-build -lcapstone -z noexecstack
INCLUDES=-I./contrib/libyaml/include -I./contrib/cJSON -I./os/$(OS) -I./contrib/pcre2/src -I./contrib/build/pcre2 -I./contrib/funchook/capstone_src/include/ -I./contrib/jni -I./contrib/jni/linux/ -I./contrib/openssl/include -I./contrib/build/openssl/include -I./contrib/build/libunwind/include -I./contrib/libunwind/include/ -I./contrib/coredumper/src

$(LIBAPPVIEW): src/wrap.c src/state.c src/httpstate.c src/metriccapture.c src/report.c src/httpagg.c src/httpmatch.c src/plattime.c src/fn.c os/$(OS)/os.c src/cfgutils.c src/cfg.c src/transport.c src/backoff.c src/log.c src/mtc.c src/circbuf.c src/linklist.c src/evtformat.c src/ctl.c src/mtcformat.c src/com.c src/appviewstdlib.c src/dbg.c src/strsearch.c src/oci.c src/wrap_go.c src/sysexec.c src/gocontext_arm.S src/appviewelf.c src/utils.c src/strset.c src/javabci.c src/javaagent.c src/ipc.c src/ipc_resp.c src/snapshot.c src/coredump.c src/evtutils.c
	@$(MAKE) -C contrib funchook pcre2 openssl ls-hpack musl libyaml libunwind cJSON coredumper
	@echo "$${CI:+::group::}Building $@"
	$(CC) $(LIBRARY_CFLAGS) $(ARCH_CFLAGS) \
		-shared -fvisibility=hidden -fno-stack-protector \
		-DAPPVIEW_VER=\"$(APPVIEW_VER)\" $(CJSON_DEFINES) $(YAML_DEFINES) \
		-pthread -o $@ $(INCLUDES) $^ $(LD_FLAGS) ${OPENSSL_AR} \
		-Wl,--version-script=libappview.map
	$(CC) -c $(LIBRARY_CFLAGS) -DAPPVIEW_VER=\"$(APPVIEW_VER)\" $(YAML_DEFINES) $(INCLUDES) $^
	$(RM) -r ./test/selfinterpose && \
		mkdir ./test/selfinterpose && \
		mv *.o ./test/selfinterpose/
	    $(RM) ./test/selfinterpose/wrap_go.o
	    $(RM) ./test/selfinterpose/gocontext_arm.o
	@[ -z "$(CI)" ] || echo "::endgroup::"
