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

        @file_label = Tk::Tile::Label.new(@main_content) {text 'File'}.grid( :row => 1, :column => 1, :sticky => 'we' )
        Tk::Tile::Label.new(@main_content) {text 'JCR:'}.grid( :row => 2, :column => 1, :sticky => 'we' )
        @jcr_text = TkText.new(@main_content) {width 80; height 10}.grid( :row => 3, :column => 1, :sticky => 'we' )
        Tk::Tile::Label.new(@main_content) {text 'Output:'}.grid( :row => 4, :column => 1, :sticky => 'we' )
        @output_text = TkText.new(@main_content) {width 80; height 10}.grid( :row => 5, :column => 1, :sticky => 'we' )
        Tk::Tile::Label.new(@main_content) {text 'Zinc:'}.grid( :row => 6, :column => 1, :sticky => 'we' )
        @zinc_text = TkText.new(@main_content) {width 80; height 10}.grid( :row => 7, :column => 1, :sticky => 'we' )

        @button_content = Tk::Tile::Frame.new(@main_content).grid( :row => 8, :column => 1, :sticky => 'ne')
        @show_all_var = TkVariable.new
        @show_all_var.value = 1
        @show_all_checkbutton = Tk::Tile::CheckButton.new( @button_content ) {text 'Show All'; }.variable( @show_all_var ).grid( :row => 1, :column => 1, :sticky => 'w' )
        Tk::Tile::Button.new(@button_content) {text '<<<'; command {zrself.prev}}.grid( :row => 2, :column => 1, :sticky => 'w' )
        Tk::Tile::Button.new(@button_content) {text '>>>'; command {zrself.next}}.grid( :row => 2, :column => 2, :sticky => 'w' )
        Tk::Tile::Button.new(@button_content) {text 'Accept'; command {zrself.accept}}.grid( :row => 3, :column => 1, :sticky => 'w' )
        Tk::Tile::Button.new(@button_content) {text 'Reject'; command {zrself.reject}}.grid( :row => 3, :column => 2, :sticky => 'w' )

        TkWinfo.children(@main_content).each {|w| TkGrid.configure w, :padx => 5, :pady => 5}
    end

    def prev
        move +1
    end

    def next
        move -1
    end

    def move dir
        @index = (@index + dir) % @test_status.size
        load_view @index
    end

    def accept
        @show_all_var.value = 1
        FileUtils.copy( @test_status[@index][:output], @test_status[@index][:zinc] ) if File.exists? @test_status[@index][:output]
        load_view @index
    end

    def reject
        @show_all_var.value = 0
        File.unlink( @test_status[@index][:zinc] )
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
        @file_label.text "File: #{@test_status[n][:jcr]}"
        @jcr_text.clear.insert 1.0, File.read( @test_status[n][:jcr] )
        @output_text.clear.insert 1.0, File.exists?( @test_status[n][:output] ) ? File.read( @test_status[n][:output] ) : ''
        @zinc_text.clear.insert 1.0, File.exists?( @test_status[n][:zinc] ) ? File.read( @test_status[n][:zinc] ) : ''
    end
end

ZincReviewer.new
