# For each sample JCR file, jcrcheck.exe is run, and the output
# recorded.  This is then compared against a 'zinc' standard.
# if one is present.  (Using 'zinc' instead of 'gold' so that
# files sort better in Explorer view!)

require_relative 'ruby-xtra/jcrglobber.rb'

class TestRunner < JCRGlobber
    def on_process_jcr jcr, output, zinc
        # puts "Testing: #{jcr}"
        File.unlink output if File.exists? output
        `#{$jcrcheck_exe} #{jcr} > #{output}`
    end

    def on_zinc_match jcr, output, zinc
        puts "OK: for #{jcr}"
    end

    def on_zinc_mismatch jcr, output, zinc
        puts "Error: Zinc mis-match for #{jcr}"
    end

    def on_no_zinc jcr, output, zinc
        puts "Warning: No Zinc file for #{jcr}"
    end

    def on_no_output jcr, output, zinc
        puts "Fault: #{output} not generated"
    end

    def report
        puts "#{error_count} error(s), #{warning_count} warning(s) in #{file_count} tests"
    end
end

def test_report
    TestRunner.new.run
end

test_report
