#!/usr/bin/env ruby

require 'set'
require 'optparse'

def matches_if(line)
    return line =~ /^ *#if (\w+) *$/ 
end

def matches_ifdef(line)
    if line =~ /^ *#ifdef (\w+) *$/ 
        $1
    else
        nil
    end
end
def matches_ifndef(line)
    if line =~ /^ *#ifndef (\w+) *$/ 
        $1
    else
        nil
    end
end
def matches_endif(line)
    line =~ /^ *#endif *$/ 
end

def matches_else(line)
    line =~ /^ *#else *$/ 
end

def openingBranch(line)
    return (matches_ifdef(line) or matches_ifndef(line) or matches_if(line))
end

class StateBasedParser
    attr_accessor :defineOn, :defineOff
    def initialize()
        @stack = [:acceptUnprocessed]
        @defineOn = Set.new
        @defineOff = Set.new
    end

    def iterate(line)
        state = @stack.last

        if state == :rejectUnprocessed
            if openingBranch(line)
                @stack << :rejectUnprocessed
            elsif matches_endif(line)
                @stack.pop
            end
            return false
        end

        if state == :reject
            if openingBranch(line)
                @stack << :rejectUnprocessed
            elsif matches_else(line)
                @stack.pop
                @stack << :accept
            elsif matches_endif(line)
                @stack.pop
            end
            return false
        end

        if state == :acceptUnprocessed
            if matches_endif(line)
                @stack.pop
                return true
            elsif matches_else(line)
                return true
            end
        end

        ifdef = matches_ifdef(line)
        if ifdef
            if @defineOn.member?(ifdef)
                @stack << :accept
                return false
            elsif @defineOff.member?(ifdef)
                @stack << :reject
                return false
            else
                @stack << :acceptUnprocessed
                return true
            end
        end

        ifndef = matches_ifndef(line)
        if ifndef
            if @defineOn.member?(ifndef)
                @stack << :reject
                return false
            elsif @defineOff.member?(ifndef)
                @stack << :accept
                return false
            else
                @stack << :acceptUnprocessed
                return true
            end
        end

        if matches_if(line)
            @stack << :acceptUnprocessed
            return true
        end

        if matches_endif(line)
            @stack.pop
            return false
        end

        if matches_else(line)
            if state == :accept
                @stack.pop
                @stack << :reject
            elsif state == :reject
                @stack.pop
                @stack << :accept
            elsif state == :acceptUnprocessed
                return true
            end
            return false
        end
        
        if state == :accept or state == :acceptUnprocessed
            return true
        else
            return false
        end
    end

    def run(file)
        output = []
        @stack = [:acceptUnprocessed]
        while not file.eof?
            line = file.readline
            accept = iterate(line)

            if $debug_output
                puts "[#{@stack * ", "}] #{line}"
            end

            if accept
                output << line
            end
        end
        output
    end
end

$parser = StateBasedParser.new
$tostdout = false
$debug_output = true

optparse = OptionParser.new do |opts|
    #opts.banner =  "Usage.."
    opts.on('-d', '--define FLAG', '') do |d|
        puts "Defining #{d}"
        $parser.defineOn.add(d)
    end
    opts.on('-u', '--undefine FLAG', '') do |d|
        puts "Undefining #{d}"
        $parser.defineOff.add(d)
    end
    opts.on('', '--stdout', '') do
        $tostdout = true
    end
end

optparse.parse!

def process_file(path)
    puts "processing file: #{path}"
    if File.directory?(path)
        Dir.foreach(path) do |file|
            if file == "." or file == ".." then next end
            file = File.join(path, file)
            process_file(file)
        end
    else
        output = $parser.run(File.new(path, 'r'))
        if $tostdout
            puts output
        else
            File.new(path, 'w').write(output * "")
        end
    end
end

ARGV.each { |path| process_file(path) }
