# For each sample JCR file, jcrcheck.exe is run, and the output
# recorded.  This is then compared against a 'gold' standard
# if one is present.

jcrcheck_exe_linux = '../bin/jcrcheck'
jcrcheck_exe_win = '..\\Debug\\jcrcheck.exe'
jcrcheck_exe = jcrcheck_exe_win     # Do something better here when we have the Internet!

Dir.glob( '*.jcr' ) do |jcr|
    output = jcr.sub( /\.jcr$/, '.txt' )
    gold = jcr.sub( /\.jcr$/, '.gold.txt' )
    File.unlink output if File.exists? output
    `#{jcrcheck_exe} #{jcr} > #{output}`
    if File.exists? output
        if File.exists? gold
            if File.read( output ) != File.read( gold )
                puts "Error: Gold mis-match for #{jcr}"
            else
                puts "OK: for #{jcr}"
            end
        else
            puts "Warning: No Gold file for #{jcr}"
        end
    else
        puts "Fault: #{output} not generated"
    end
end
