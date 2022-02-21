import subprocess
benchmarks = ['619.lbm_s', '538.imagick_r', '644.nab_s', '625.x264_s', '605.mcf_s', '657.xz_s', '602.gcc_s', '600.perlbench_s', 
    '631.deepsjeng_s', '641.leela_s', '623.xalancbmk_s']

REPLACEMENT_POLICIES = ['NMRURP', 'RANDOMRP', 'LIPRP']

for REPLACEMENT_POLICY in REPLACEMENT_POLICIES:
    bashCommand = "rm -rf m5out/"
    process = subprocess.Popen(bashCommand.split(), stdout=subprocess.PIPE)
    output, error = process.communicate()
    bashCommand = "make simall REPLACEMENT_POLICY=" + REPLACEMENT_POLICY
    bashCommand += "CACHE_ASSOC=8"
    process = subprocess.Popen(bashCommand.split(), stdout=subprocess.PIPE)
    output, error = process.communicate()
    for benchmark in benchmarks:
        bashCommand = "cp -a " + "benchspec/CPU/"
        bashCommand+= benchmark + "/run/run_base_refspeed_mytest-m64.0000/m5out/"
        bashCommand+= "../hw3-results/" + REPLACEMENT_POLICY + "/" + benchmark + "/"
        process = subprocess.Popen(bashCommand.split(), stdout=subprocess.PIPE)
        output, error = process.communicate()

