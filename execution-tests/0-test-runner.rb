# For each sample JCR file, jcrcheck.exe is run, and the output
# recorded.  This is then compared against a 'gold' standard
# if one is present.

jcrcheck_exe_linux = '../bin/jcrcheck'
jcrcheck_exe_win = '..\\Debug\\jcrcheck.exe'
jcrcheck_exe = jcrcheck_exe_win     # Do something better here when we have the Internet!

test_count = 0
warning_count = 0
error_count = 0

Dir.glob( '*.jcr' ) do |jcr|
    output = jcr.sub( /\.jcr$/, '.txt' )
    gold = jcr.sub( /\.jcr$/, '.gold.txt' )
    File.unlink output if File.exists? output
    `#{jcrcheck_exe} #{jcr} > #{output}`
    test_count += 1
    if File.exists? output
        if File.exists? gold
            if File.read( output ) != File.read( gold )
                puts "Error: Gold mis-match for #{jcr}"
                error_count += 1
            else
                puts "OK: for #{jcr}"
            end
        else
            puts "Warning: No Gold file for #{jcr}"
            warning_count += 1
        end
    else
        puts "Fault: #{output} not generated"
        error_count += 1
    end
end

puts "#{error_count} error(s), #{warning_count} warning(s) in #{test_count} tests"
