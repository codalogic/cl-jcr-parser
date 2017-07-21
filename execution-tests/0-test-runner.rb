# For each sample JCR file, jcrcheck.exe is run, and the output
# recorded.  This is then compared against a 'zinc' standard.
# if one is present.  (Using 'zinc' instead of 'gold' so that
# files sort better in Explorer view!)

jcrcheck_exe = ''
jcrcheck_exe_linux = '../bin/jcrcheck'
jcrcheck_exe_win = '..\\Debug\\jcrcheck.exe'
jcrcheck_exe = jcrcheck_exe_linux if File.exists? jcrcheck_exe_linux    # Necessary, but not sufficient for running on Linux
jcrcheck_exe = jcrcheck_exe_win if File.exists?( jcrcheck_exe_win ) && `#{jcrcheck_exe_win} 2>&1` =~ /jcrcheck/
if jcrcheck_exe.empty?
    puts "Error: No jcrcheck program found.  This will need to be built before these tests can be run"
    exit
end

test_count = 0
warning_count = 0
error_count = 0

Dir.glob( '*.jcr' ) do |jcr|
    output = jcr.sub( /\.jcr$/, '.txt' )
    zinc = jcr.sub( /\.jcr$/, '.zinc.txt' )
    File.unlink output if File.exists? output
    `#{jcrcheck_exe} #{jcr} > #{output}`
    test_count += 1
    if File.exists? output
        if File.exists? zinc
            if File.read( output ) != File.read( zinc )
                puts "Error: Zinc mis-match for #{jcr}"
                error_count += 1
            else
                puts "OK: for #{jcr}"
            end
        else
            puts "Warning: No Zinc file for #{jcr}"
            warning_count += 1
        end
    else
        puts "Fault: #{output} not generated"
        error_count += 1
    end
end

puts "#{error_count} error(s), #{warning_count} warning(s) in #{test_count} tests"
