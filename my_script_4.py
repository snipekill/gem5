import subprocess
benchmarks = ['619.lbm_s', '638.imagick_s', '644.nab_s', '625.x264_s', '605.mcf_s', '657.xz_s', '602.gcc_s', '600.perlbench_s', 
    '631.deepsjeng_s', '641.leela_s', '623.xalancbmk_s']
# benchmarks = ['638.imagick_s']

# REPLACEMENT_POLICIES = ['NMRURP', 'RandomRP', 'LIPRP']
REPLACEMENT_POLICIES = ['LIPRP']
# REPLACEMENT_POLICIES = ['NMRURP', 'LIPRP']

for REPLACEMENT_POLICY in REPLACEMENT_POLICIES:
    bashCommand = "make simall -j11 REPLACEMENT_POLICY=" + REPLACEMENT_POLICY
    bashCommand += " CACHE_ASSOC=4"
    print(bashCommand)
    process = subprocess.Popen(bashCommand.split(), stdout=subprocess.PIPE)
    output, error = process.communicate()
    for benchmark in benchmarks:
        bashCommand = "cp -a " + "benchspec/CPU/"
        bashCommand+= benchmark + "/run/run_base_refspeed_mytest-m64.0000/m5out/"
        bashCommand+= " ../hw3-results/LIPRP-4/" + benchmark + "/"
        print(bashCommand)
        process = subprocess.Popen(bashCommand.split(), stdout=subprocess.PIPE)
        output, error = process.communicate()