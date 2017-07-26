# For each sample JCR file, jcrcheck.exe is run, and the output
# recorded.  This is then compared against a 'zinc' standard.
# if one is present.  (Using 'zinc' instead of 'gold' so that
# files sort better in Explorer view!)

require 'os'    # May need to do 'gem install os' (If your gems installation is old, you may need to re-install gems to get latest certificates)

$jcrcheck_exe = OS.windows? ? '..\\Debug\\jcrcheck.exe' : '../bin/jcrcheck'
if not File.exists? $jcrcheck_exe
    puts "Error: No jcrcheck program found.  This will need to be built before these tests can be run"
    exit
end

class JCRGlobber
    attr_reader :file_count, :warning_count, :error_count

    def initialize
        @file_count = @warning_count = @error_count = 0
    end

    def run
        Dir.glob( '*' ).select { |name| File.directory? name }.each do |dir|
            Dir.glob( "#{dir}/*.jcr" ) do |jcr|
                @file_count += 1
                is_zinc_match = is_zinc_present = is_output_present = false
                output = jcr.sub( /\.jcr$/, '.txt' )
                zinc = jcr.sub( /\.jcr$/, '.zinc.txt' )
                on_process_jcr jcr, output, zinc
                if File.exists? output
                    is_output_present = true
                    if File.exists? zinc
                        is_zinc_present = true
                        if File.read( output ) == File.read( zinc )
                            on_zinc_match jcr, output, zinc
                            is_zinc_match = true
                        else
                            @error_count += 1
                            on_zinc_mismatch jcr, output, zinc
                        end
                    else
                        @warning_count += 1
                        on_no_zinc jcr, output, zinc
                    end
                else
                    @error_count += 1
                    on_no_output jcr, output, zinc
                end
                on_result( { jcr: jcr, output: output, zinc: zinc,
                            is_zinc_match: is_zinc_match,
                            is_zinc_present: is_zinc_present,
                            is_output_present: is_output_present } )
            end
        end
        report
    end

    def on_process_jcr jcr, output, zinc
    end

    def on_zinc_match jcr, output, zinc
    end

    def on_zinc_mismatch jcr, output, zinc
    end

    def on_no_zinc jcr, output, zinc
    end

    def on_no_output jcr, output, zinc
    end

    def on_result state
    end

    def report
    end
end
