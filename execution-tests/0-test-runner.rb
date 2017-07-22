# For each sample JCR file, jcrcheck.exe is run, and the output
# recorded.  This is then compared against a 'zinc' standard.
# if one is present.  (Using 'zinc' instead of 'gold' so that
# files sort better in Explorer view!)

require 'os'    # May need to do 'gem install os' (If your gems installation is old, you may need to re-install gems to get latest certificates)

jcrcheck_exe = OS.windows? ? '..\\Debug\\jcrcheck.exe' : '../bin/jcrcheck'
if not File.exists? jcrcheck_exe
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
