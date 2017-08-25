require 'tk'
require 'tkextlib/tile'
require_relative 'ruby-xtra/jcrglobber.rb'
require 'fileutils'

class ZincReviewer

    def initialize
        initialize_window_laout
        read_test_status
        show_first_file
        Tk.mainloop     # Must be last configuration option as after this control passes to TK
    end

    private def initialize_window_laout
        zrself = self
        @root = TkRoot.new {title "Zinc Reviewer"}
        @main_content = Tk::Tile::Frame.new(@root) {padding "3 3 12 12"}.grid( :sticky => 'nsew')
        TkGrid.columnconfigure @main_content, 0, :weight => 1
        TkGrid.rowconfigure @main_content, 0, :weight => 1

        @file_label = Tk::Tile::Label.new(@main_content) {text 'File'}.
                                    grid( :row => 1, :column => 1, :sticky => 'we' )
        Tk::Tile::Label.new(@main_content) {text 'JCR:'}.
                                    grid( :row => 2, :column => 1, :sticky => 'we' )
        @jcr_text = TkText.new(@main_content) {width 80; height 10}.
                                    grid( :row => 3, :column => 1, :sticky => 'we' )
        Tk::Tile::Label.new(@main_content) {text 'Output:'}.
                                    grid( :row => 4, :column => 1, :sticky => 'we' )
        @output_text = TkText.new(@main_content) {width 80; height 10}.
                                    grid( :row => 5, :column => 1, :sticky => 'we' )


        @button_content = Tk::Tile::Frame.new(@main_content).
                                    grid( :row => 6, :column => 1, :sticky => 'nw')
            @prev_button = Tk::Tile::Button.new(@button_content) {text '<<<'; command {zrself.prev}}.
                                        grid( :row => 1, :column => 1, :sticky => 'w' )
            @next_button = Tk::Tile::Button.new(@button_content) {text '>>>'; command {zrself.next}}.
                                        grid( :row => 1, :column => 2, :sticky => 'w' )
            @accept_button = Tk::Tile::Button.new(@button_content) {text 'Accept'; command {zrself.accept}}.
                                        grid( :row => 2, :column => 1, :sticky => 'w' )
            @reject_button = Tk::Tile::Button.new(@button_content) {text 'Reject'; command {zrself.reject}}.
                                        grid( :row => 2, :column => 2, :sticky => 'w' )

        @show_only_tests_failing_zinc_var = TkVariable.new
        @show_only_tests_failing_zinc_var.value = 0
        @show_all_checkbutton = Tk::Tile::CheckButton.new( @main_content ) {text 'Show Only Tests Failing Zinc Reference'; }.
                                    variable( @show_only_tests_failing_zinc_var ).
                                    grid( :row => 6, :column => 1, :sticky => 'ne' )

        Tk::Tile::Label.new(@main_content) {text 'Zinc:'}.
                                    grid( :row => 7, :column => 1, :sticky => 'we' )
        @zinc_text = TkText.new(@main_content) {width 80; height 10}.
                                    grid( :row => 8, :column => 1, :sticky => 'we' )

        TkWinfo.children(@main_content).each {|w| TkGrid.configure w, :padx => 5, :pady => 5}
    end

    def prev
        move -1
    end

    def next
        move +1
    end

    def move dir
        limit = dir < 0 ? 0 : @test_status.size - 1
        if limit == @index
            Tk::messageBox :message => 'At end of test selection', :title => 'Select Results'

        elsif @show_only_tests_failing_zinc_var.value.to_i == 0
            @index = (@index + dir) % @test_status.size
            load_view @index

        else
            i = @index
            loop do
                i = (i + dir) % @test_status.size
                if ! @test_status[i][:is_zinc_present] or ! @test_status[i][:is_zinc_match]
                    @index = i
                    load_view @index
                    break
                end
                if i == limit
                    Tk::messageBox :message => 'No more failed tests for review', :title => 'Select Results'
                    break
                end
            end
        end
    end

    def accept
        if File.exists? @test_status[@index][:output]
            FileUtils.copy( @test_status[@index][:output], @test_status[@index][:zinc] )
            @test_status[@index][:is_zinc_match] = @test_status[@index][:is_zinc_present] = true
        end
        load_view @index
    end

    def reject
        File.unlink( @test_status[@index][:zinc] )
        @test_status[@index][:is_zinc_match] = @test_status[@index][:is_zinc_present] = false
        load_view @index
    end
    
    def read_test_status
        @test_status = JCRGlobber.new.run
    end
    
    def show_first_file
        @index = 0
        load_view @index
    end
    
    def load_view n
        @file_label.text "File #{n+1}: #{@test_status[n][:jcr]}"
        @jcr_text.clear.insert 1.0, File.read( @test_status[n][:jcr] )
        @output_text.clear.insert 1.0, File.exists?( @test_status[n][:output] ) ? File.read( @test_status[n][:output] ) : '<No File>'
        @zinc_text.clear.insert 1.0, File.exists?( @test_status[n][:zinc] ) ? File.read( @test_status[n][:zinc] ) : '<No File>'
        @prev_button.state (n==0) ? "disabled" : "normal"
        @next_button.state (n>=@test_status.size-1) ? "disabled" : "normal"
        @accept_button.state (File.exists?( @test_status[@index][:output] ) && ! @test_status[@index][:is_zinc_match] ) ? "normal" : "disabled"
        @reject_button.state (File.exists? @test_status[@index][:zinc]) ? "normal" : "disabled"
    end
end

ZincReviewer.new
