#!/usr/bin/python

# The MIT License (MIT)
#
# Copyright (c) 2016 Codalogic Ltd
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Exodep is a simple dependency downloader. See the following for more details:
#
#     https://github.com/codalogic/exodep

import sys
import re
import io
import os
import urllib.request
import tempfile
import shutil
import filecmp

host_templates = {
        'github': 'https://raw.githubusercontent.com/${owner}/${project}/${strand}/${path}${file}',
        'bitbucket': 'https://bitbucket.org/${owner}/${project}/raw/${strand}/${path}${file}' }

def main() :
    ProcessDeps( sys.argv[1] if len( sys.argv ) >= 2 else "mydeps.exodep" )

class ProcessDeps:
    are_any_files_changed = False

    def __init__( self, dependencies_src, vars = { 'strand': 'master', 'path': '' } ):
        self.are_files_changed = False
        self.line_num = 0
        self.uritemplate = host_templates['github']
        self.vars = vars.copy()
        self.versions = {}  # Each entry is <string of space separated strand names> : <string to use as strand in uri template>
        if isinstance( dependencies_src, str ):
            if self.is_already_processed( dependencies_src ):
                return
            self.file = dependencies_src
            self.process_dependency_file()
        elif isinstance( dependencies_src, io.StringIO ):    # Primarily for testing
            self.file = "<StringIO>"
            self.process_dependency_stream( dependencies_src )
        else:
            self.error( "Unrecognised dependencies_src type format" )

    processed_dependencies = {}

    def is_already_processed( self, dependencies_src ):
        abs_dependencies_src = os.path.abspath( dependencies_src )
        if abs_dependencies_src in ProcessDeps.processed_dependencies:
            return True
        ProcessDeps.processed_dependencies[abs_dependencies_src] = True
        return False

    def process_dependency_file( self ):
        try:
            with open(self.file) as f:
                self.process_dependency_stream( f )
        except FileNotFoundError:
            self.error( "Unable to open exodep file: " + self.file )

    def process_dependency_stream( self, f ):
        for line in f:
            self.line_num += 1
            self.process_line( line )

    def process_line( self, line ):
        line = line.rstrip()
        line = remove_comments( line )
        if is_blank_line( line ):
            return
        if not (self.consider_include( line ) or
                self.consider_sinclude( line ) or
                self.consider_hosting( line ) or
                self.consider_uri_template( line ) or
                self.consider_versions( line ) or
                self.consider_variable( line ) or
                self.consider_default_variable( line ) or
                self.consider_copy( line ) or
                self.consider_bcopy( line ) or
                self.consider_file_ops( line ) or
                self.consider_exec( line ) or
                self.consider_subst( line ) or
                self.consider_on_conditional( line ) or
                self.consider_onchanged( line ) or
                self.consider_onanychanged( line ) or
                self.consider_os_conditional( line ) ):
            self.report_unrecognised_command( line )

    def consider_include( self, line ):
        m = re.match( 'include\s+(.+)', line )
        if m != None:
            file_name = self.script_relative_path( m.group(1) )
            if not os.path.isfile( file_name ):
                self.error( "'include' file not found: " + file_name )
                return True     # It was an 'include' command, even though it was a bad one
            ProcessDeps( file_name, self.vars )
            return True
        return False

    def consider_sinclude( self, line ):        # This method is used to support testing
        m = re.match( 'sinclude\s+(.+)', line )
        if m != None:
            args = m.group(1)
            ProcessDeps( io.StringIO( args.replace( '\t', '\n' ) ), self.vars )
            return True
        return False

    def script_relative_path( self, src ):
        return os.path.normpath( os.path.join( os.path.dirname( self.file ), src ) ).replace( '\\', '/' )

    def consider_hosting( self, line ):
        m = re.match( 'hosting\s+(.+)', line )
        if m != None:
            host = m.group(1)
            if host in host_templates:
                self.uritemplate = host_templates[host]
            else:
                self.error( "Unrecognised hosting server provider: " + host )
            return True
        return False

    def consider_uri_template( self, line ):
        m = re.match( 'uritemplate\s+(.+)', line )
        if m != None:
            self.uritemplate = m.group(1)
            return True
        return False

    def consider_versions( self, line ):
        m = re.match( 'versions(?:\s+(.*))?', line )    # If no arguments to 'versions' command, it should use 'versions.exodep'
        if m != None:
            file_name = m.group(1) if m.group(1) else 'versions.exodep'
            uri = self.make_master_strand_uri( file_name )
            try:
                if re.match( 'https?://', uri ):
                    with urllib.request.urlopen( uri ) as fin:
                        self.parse_versions_info( fin )
                else:
                    with open( uri, "rt" ) as fin:
                        self.parse_versions_info( fin )
            except:
                self.error( "Unable to retrieve: " + uri )
            return True
        return False

    def parse_versions_info( self, fin ):
        for line in fin:
            if isinstance( line, bytes ):
                line = line.decode( 'utf-8' )
            line = line.rstrip()
            line = remove_comments( line )
            if not is_blank_line( line ):
                m = re.match( '^(\S+)\s+(.*)', line )
                if m != None:
                    self.versions[m.group(2)] = m.group(1)

    def consider_variable( self, line ):
        m = re.match( '\$(\w+)(?:\s+(.*))?$', line )
        if m != None:
            self.vars[m.group(1)] = m.group(2) if m.group(2) else ''
            return True
        return False

    def consider_default_variable( self, line ):
        m = re.match( 'default\s+\$(\w+)\s+(.*)', line )
        if m != None:
            if m.group(1) not in self.vars:
                self.vars[m.group(1)] = m.group(2)
            return True
        return False

    def consider_copy( self, line ):
        m = re.match( 'copy\s+(\S+)(?:\s+(\S+))?', line )
        if m != None:
            self.retrieve_text_file( m.group(1), m.group(2) )
            return True
        return False

    def consider_bcopy( self, line ):
        m = re.match( 'bcopy\s+(\S+)(?:\s+(\S+))?', line )
        if m != None:
            self.retrieve_binary_file( m.group(1), m.group(2) )
            return True
        return False

    def report_unrecognised_command( self, line ):
        self.error( "Unrecognised command: " + line )

    def retrieve_text_file( self, src, dst ):
        self.retrieve_file( src, dst, TextDownloadHandler() )

    def retrieve_binary_file( self, src, dst ):
        self.retrieve_file( src, dst, BinaryDownloadHandler() )

    def retrieve_file( self, src, dst, handler ):
        if dst == None:
            if re.match( 'https?://', src ):
                self.error( "Explicit uri not supported with commands of the form 'bcopy src_and_dst'" )
                return
            dst = src
            if re.search( '\$\{path\}', self.uritemplate ):
                dst = self.vars['path'] + dst
        from_uri = self.make_uri( src )
        to_file = self.make_destination_file_name( src, dst )
        if from_uri == '':
            self.error( "Unable to evaluate source of: " + src )
            return
        if to_file == '':
            self.error( "Unable to evaluate destination of: " + dst )
            return
        if re.match( 'https?://', from_uri ):
            tmp_name = handler.download_to_temp_file( from_uri )
        else:
            tmp_name = self.local_copy_to_temp_file( from_uri )     # Taking a local copy is not optimal, but keeps the subsequent update logic the same
        if not tmp_name:
            self.error( "Unable to retrieve: " + from_uri )
            return
        self.conditionally_update_dst_file( tmp_name, to_file )

    def conditionally_update_dst_file( self, tmp_name, to_file ):
        if not os.path.isfile( to_file ):
            if os.path.dirname( to_file ):
                os.makedirs( os.path.dirname( to_file ), exist_ok=True )
            shutil.move( tmp_name, to_file )
            self.are_files_changed = ProcessDeps.are_any_files_changed = True
            print( 'Created...', to_file )
        elif not filecmp.cmp( tmp_name, to_file ):
            shutil.move( tmp_name, to_file )
            self.are_files_changed = ProcessDeps.are_any_files_changed = True
            print( 'Updated...', to_file )
        else:
            os.unlink( tmp_name )
            print( 'Same......', to_file )

    def make_master_strand_uri( self, file_name ):
        # Override ${master} and ${path} variable
        uri = re.compile( '\$\{strand\}' ).sub( 'master', self.uritemplate )
        uri = re.compile( '\$\{path\}' ).sub( '', uri )
        return self.make_uri( file_name, uri )

    def make_uri( self, file_name, uri = None ):
        if re.match( 'https?://', file_name ):
            return self.expand_variables( file_name )
        if uri == None:
            uri = self.uritemplate
        uri = re.compile( '\$\{file\}' ).sub( file_name, uri )
        return self.expand_variables( uri )

    def make_destination_file_name( self, src, dst ):
        # dst in a copy command may refer to a folder, in which case the base file name from the src needs to be incorporated
        dst = self.expand_variables( dst )
        if dst.endswith( '/' ) or os.path.isdir( dst ):
            if not dst.endswith( '/' ):
                dst = dst + '/'
            return dst + os.path.basename( src )
        return dst

    def expand_variables( self, uri ):
        while( True ):
            m = re.search( '\$\{(\w+)\}', uri )
            if m == None:
                return uri
            var_name = m.group(1)
            if var_name == 'strand':        # The strand variable has 'magic' properties (it can be mutated by a versions file) so we need to do something special with it
                uri = re.compile( '\$\{strand\}' ).sub( self.select_strand(), uri )
            elif var_name in self.vars:
                uri = re.compile( '\$\{' + var_name + '\}' ).sub( self.vars[var_name], uri )
            else:
                self.error( "Unrecognised substitution variable: " + var_name )
                return ''

    def select_strand( self ):
        if 'strand' not in self.vars:
            self.error( "No suitable 'strand' variable available for substitution" )
            return ''
        strand = self.vars['strand']
        for supported_strands in self.versions:
            if strand in supported_strands.split():
                return self.versions[supported_strands]
        return self.vars['strand']

    def local_copy_to_temp_file( self, file ):
        try:
            fout = None
            with open( file, 'rb' ) as fin:
                fout = tempfile.NamedTemporaryFile( mode='wb', delete=False )
                while True:
                    data = fin.read( 1000 )
                    if not data:
                        break
                    fout.write( data )
            fout.close()
            return fout.name
        except FileNotFoundError:
            return ''

    def consider_subst( self, line ):
        m = re.match( 'subst\s+(\S+)(?:\s+(\S+))?', line )
        if m != None:
            src = m.group(1)
            dst = m.group(2)
            if dst == None:
                dst = src
            try:
                fout = None
                with open( src, 'rt' ) as fin:
                    fout = tempfile.NamedTemporaryFile( mode='wt', delete=False )
                    for line in fin:
                        fout.write( self.subst_expand_variables( line ) )
                fout.close()
                self.conditionally_update_dst_file( fout.name, dst )
            except FileNotFoundError:
                error( "Unable to open file for 'subst' command: " + src )
            return True
        return False

    def subst_expand_variables( self, line ):
        while( True ):
            m = re.search( '\$\{exodep:(\w+)\}', line )
            if m == None:
                return line
            var_name = m.group(1)
            if var_name == 'strand':        # The strand variable has 'magic' properties (it can be mutated by a versions file) so we need to do something special with it
                line = re.compile( '\$\{strand\}' ).sub( self.select_strand(), line )
            elif var_name in self.vars:
                line = re.compile( '\$\{exodep:' + var_name + '\}' ).sub( self.vars[var_name], line )
            else:
                self.error( "Unrecognised variable in 'subst' command: " + var_name )
                return line

    def consider_file_ops( self, line ):
        m = re.match( '(cp|mv)\s+(\S+)\s+(\S+)', line )
        if m != None:
            op = m.group(1)
            src = self.expand_variables( m.group(2) )
            dst = self.expand_variables( m.group(3) )
            if op == 'cp':
                try:
                    shutil.copy( src, dst )
                except:
                    self.error( "Unable to 'cp' file '" + src + "' to '" + dst + "'" )
            elif op == 'mv':
                try:
                    shutil.move( src, dst )
                except:
                    self.error( "Unable to 'mv' file '" + src + "' to '" + dst + "'" )
            return True
        m = re.match( '(mkdir|rmdir|rm)\s+(\S+)', line )
        if m != None:
            op = m.group(1)
            path = self.expand_variables( m.group(2) )
            if op == 'mkdir':
                try:
                    os.makedirs( path, exist_ok=True )
                except:
                    self.error( "Unable to 'mkdir' for '" + path + "'" )
            elif op == 'rmdir':
                try:
                    shutil.rmtree( path )
                except:
                    self.error( "Unable to 'rmdir' on '" + path + "'" )
            elif op == 'rm':
                try:
                    os.unlink( path )
                except:
                    self.error( "Unable to 'rm' file '" + path + "'" )
            return True
        return False

    def consider_exec( self, line ):
        m = re.match( 'exec\s+(.+)', line )
        if m != None:
            command = m.group(1)
            org_cwd = os.getcwd()
            file_dirname = os.path.dirname( self.file )
            if file_dirname:
                os.chdir( file_dirname )
            os.system( self.expand_variables( command ) )
            os.chdir( org_cwd )
            return True
        return False

    def consider_on_conditional( self, line ):
        m = re.match( 'on\s+\$(\w+)\s+(.+)', line )
        if m != None:
            var_name = m.group(1)
            command = m.group(2)
            if var_name in self.vars and self.vars[var_name] != '':
                self.process_line( command )
            return True
        return False

    def consider_onchanged( self, line ):
        m = re.match( 'onchanged\s+(.+)', line )
        if m != None:
            command = m.group(1)
            if self.are_files_changed:
                self.process_line( command )
            return True
        return False

    def consider_onanychanged( self, line ):
        m = re.match( 'onanychanged\s+(.+)', line )
        if m != None:
            command = m.group(1)
            if ProcessDeps.are_any_files_changed:
                self.process_line( command )
            return True
        return False

    os_names = { 'windows': 'win32', 'linux': 'linux', 'osx': 'darwin' }

    def consider_os_conditional( self, line ):
        m = re.match( '(windows|linux|osx)\s+(.+)', line )
        if m != None:
            os_key = m.group(1)
            command = m.group(2)
            if sys.platform.startswith( ProcessDeps.os_names[os_key] ):
                self.process_line( command )
            return True
        return False

    def error( self, what ):
        print( "Error:", self.file + ", line " + str(self.line_num) + ":" )
        print( "      ", what )

def remove_comments( line ):
    return re.compile( '\s*#.*' ).sub( '', line )

def is_blank_line( line ):
    return re.match( '^\s*$', line )

class TextDownloadHandler:
    def download_to_temp_file( self, uri ):
        try:
            fout = None
            with urllib.request.urlopen( uri ) as fin:
                fout = tempfile.NamedTemporaryFile( mode='wt', delete=False )
                for line in fin:
                    fout.write( self.normalise_line_ending( line.decode('utf-8') ) )
            fout.close()
            return fout.name
        except urllib.error.URLError:
            return ''

    def normalise_line_ending( self, line ):
        org_len = len( line )
        line = line.rstrip( '\r\n' )
        if org_len > len( line ):
            line = line + '\n'
        return line

class BinaryDownloadHandler:
    def download_to_temp_file( self, uri ):
        try:
            fout = None
            with urllib.request.urlopen( uri ) as fin:
                fout = tempfile.NamedTemporaryFile( mode='wb', delete=False )
                while True:
                    data = fin.read( 1000 )
                    if not data:
                        break
                    fout.write( data )
            fout.close()
            return fout.name
        except urllib.error.URLError:
            return ''

if __name__ == "__main__":
    main()
