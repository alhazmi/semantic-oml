# Copyright (c) 2009 - 2012 National ICT Australia Limited (NICTA).
# This software may be used and distributed solely under the terms of the MIT license (License).
# You should find a copy of the License in LICENSE.TXT or at http://opensource.org/licenses/MIT.
# By downloading or using this software you accept the terms and the liability disclaimer in the License.

# Modified by Andisa Dewi, Yahya Al-Hazmi, Technische Universitaet Berlin, 2015
# ------------------
#
# = oml4r.rb
#
# == Description
#
# This is a simple client library for OML which does not use liboml2 and its
# filters, but connects directly to the server using the +text+ protocol.
# User can use this library to create ruby applications which can send
# measurement to the OML collection server.
#

require 'set'
require 'socket'
require 'monitor'
require 'thread'
require 'optparse'
require 'securerandom'

require 'oml4r/version'


#
# This is the OML4R module, which should be required by ruby applications
# that want to collect measurements via OML
#
module OML4R

  DEF_SERVER_PORT = 3003
  DEF_PROTOCOL = 4

  # Overwrite the default logger
  #
  # @param logger Needs to respond to 'debug', 'info', ...
  #
  def self.logger=(logger)
    @@logger = logger
  end

  class OML4RException < Exception; end
  class MissingArgumentException < OML4RException; end
  class ArgumentMismatchException < OML4RException; end
  #
  # Measurement Point Class
  # Ruby applications using this module should sub-class this MPBase class
  # to define their own Measurement Point (see the example at the end of
  # this file)
  #
  class MPBase

    # Some Class variables
    @@appName = nil
    @@defs = {}
    @@channels = {}
    @@channelNames = {}
    @@frozen = false
    @@useOML = false
    @@start_time = nil
    @@newDefs = Set[]

    # Execute a block for each defined MP
    def self.each_mp(&block)
      @@defs.each(&block)
    end

    def self.has_mp?(name)
      exist = false
      @@defs.each { |m| exist = true if m[0].__def__[:name][:name]==name.to_sym }
      exist
    end

    # Set the useOML flag. If set to false, make 'inject' a NOOP
    def self.__useOML__()
      @@useOML = true
    end

    # Returns the definition of this MP
    def self.__def__()
      unless (defs = @@defs[self])
        defs = @@defs[self] = {}
        defs[:p_def] = []
        defs[:seq_no] = 0
        defs[:meta_no] = 0
      end
      defs
    end

    # Set a name for this MP
    #
    # param opts Options
    # opts add_prefix Add app name as prefix to table. Default: true
    #
    def self.name(name, opts = {})

      # ok, lets add it then
      if opts[:add_prefix].nil?
        opts[:add_prefix] = true
      end
      __def__()[:name] = {:name => name, :opts => opts}

      # if we're frozen remember to inject schema before measurements
      if @@frozen
        @@newDefs.add self
      end

    end

    # Set the channel these measurements should be sent out on.
    # Multiple declarations are allowed, and ':default' identifies
    # the channel defined by the command line arguments or environment variables.
    #
    def self.channel(channel, domain = :default)
      (@@channelNames[self] ||= []) << [channel, domain]
    end

    # Set a metric for this MP
    # - name = name of the metric to set
    # - opts = a Hash with the options for this metric
    #          Only supported option is :type = { :string | :int32 | :uint32 | :int64 | :uint64 | :double | :bool | :guid |
    #          [ DEPRECATED :long | :integer ] }
    def self.param(name, opts = {})
      o = opts.dup
      o[:name] = name
      o[:type] ||= :string
      if :long == o[:type] or :integer == o[:type]
	# XXX: :name in :name... See #1527 bullet point 3
        OML4R.logger.warn "Type #{o[:type]} for #{__def__()[:name][:name]}.#{o[:name]} is deprecated, use :int32 instead"
        o[:type] = :int32
      end
  
      __def__()[:p_def] << o
      nil
    end

    # Inject a metadata measurement from this Measurement Point ot the OML Server. 
    # - key = a string used to identify the key
    # - value = the string value
    # - fname = when not nil a string used to qualify the subject name
    def self.inject_metadata(key, value, fname = nil)

      return unless @@useOML

      # retrieve infos
      defs = @@defs[self]
      mp_name_def = defs[:name]
      mp_name = mp_name_def[:name]
      pdefs = defs[:p_def]
      defs[:types] = pdefs.map {|h| h[:type]}

      # construct the subject reference
      subject = "."
      if self != OML4R::ExperimentMetadata
        subject +=  "#{@@appName}_#{mp_name}"
        unless fname.nil?
          subject += ".#{fname}"
        end
      end

      # prepare the message header
      a = []
      a << Time.now - @@start_time
      a << "0"
      a << (defs[:meta_no] += 1)
      a << subject
      a << key
      a << value
      msg = a.join("\t")

      # Setup channels for the ExperimentMetadata MP
      chans = @@channels[self] || []
      if chans.empty?
        @@channels[self] = [[Channel[], 0]]
      end

      # now inject the schema
      @@channels[self].each do |ca|
        channel = ca[0]
        index = ca[1]
        channel.send msg
      end
    end

    # Inject a measurement from this Measurement Point to the OML Server
    # However, if useOML flag is false, then only prints the measurement on stdout
    # - args = a list of arguments (comma separated) which correspond to the
    #          different values of the metrics for the measurement to inject
    def self.inject(*args)
      return unless @@useOML

      defs = __def__()

      # Do we need to send the schema?
      if @@newDefs.include? self
        # Identify MP details
        mp_name_def = defs[:name]
        mp_name = mp_name_def[:name]
        pdefs = defs[:p_def]
        defs[:types] = pdefs.map {|h| h[:type]}
        defs[:relation] = pdefs.map {|h| h[:relation]}
        # Setup channel and schema
        channel = Channel[]
        schema_info = channel.build_schema(mp_name, mp_name_def[:opts][:add_prefix], pdefs)
        @@channels[self] = [[channel, schema_info[0]]]
        # Inject it!
        ExperimentMetadata.inject_metadata("schema", schema_info[1])
        @@newDefs.delete self
      end

      # Check that the list of values passed as argument matches the
      # definition of this Measurement Point
      pdef = defs[:p_def]
      types = defs[:types]
      if args.size != pdef.size
        raise ArgumentMismatchException.new "OML4R: Size mismatch between the measurement (#{args.size}) and the MP definition (#{pdef.size})!"
      end

      # Now prepare the measurement...
      t = Time.now - @@start_time
      a = []
      a << (defs[:seq_no] += 1)
      args.each_with_index do |arg, i|
        case types[i]
        when :double
          arg = "NaN" if arg.nil?
        when :string
          # Escape tabs and newlines
          arg = arg.to_s.gsub("\\", "\\\\").gsub("\r", "\\r").gsub("\n", "\\n").gsub("\t", "\\t")
        when :bool
          # Convert boolean value to appropriate literal
          arg = arg ? "True" : "False"
        when :blob
          arg = [arg].pack("m")
        end
        a << arg
      end
      # ...and inject it!
      msg = a.join("\t")
      @@channels[self].each do |ca|
        channel = ca[0]
        index = ca[1]
        channel.send "#{t}\t#{index}\t#{msg}"
      end

      args
    end

    # Inject measurement metadata from this Measurement Point to the
    # OML Server.
    # 
    # def self.inject_metadata(key, value, fname)
    #    MPBase::__inject_metadata__(@name, key, value, fname)
    # end

    def self.start_time()
      @@start_time
    end

    # Freeze the definition of further MPs
    #
    def self.__freeze__(appName, start_time)
      return if @@frozen
      @@frozen = true
      @@appName = appName
      # create type array for easier processing in inject
      @@defs.each do |name, descr|
        descr[:types] = descr[:p_def].map {|h| h[:type]}
      end

      # replace channel names with channel object
      self.each_mp do |klass, defs|
        names = @@channelNames[klass] || []
        OML4R.logger.debug "'#{names.inspect}', '#{klass}'"
        chans = names.collect do |cname, domain|
          # return it in an array as we need to add the channel specific index
          [Channel[cname.to_sym, domain.to_sym]]
        end
        OML4R.logger.debug "Using channels '#{chans.inspect}"
        @@channels[klass] = chans.empty? ? [[Channel[]]] : chans
      end
      @@start_time = start_time
    end

    def self.__unfreeze__()
      self.each_mp do |klass, defs|
        defs[:seq_no] = 0
      end
      @@channels = {}
      @@start_time = nil
      @@frozen = false
    end

    # Build the table schema for this MP and send it to the OML collection server
    # - name_prefix = the name for this MP to use as a prefix for its table
    def self.__print_meta__(name_prefix = nil)
      return unless @@frozen
      defs = __def__()

      # Do some sanity checks...
      unless (mp_name_def = defs[:name])
        raise MissingArgumentException.new "Missing 'name' declaration for '#{self}'"
      end

      # Build the schema
      mp_name = mp_name_def[:name]
      @@channels[self].each do |ca|
        OML4R.logger.debug "Setting up channel '#{ca.inspect}"
        schema_info = ca[0].build_schema(mp_name, mp_name_def[:opts][:add_prefix], defs[:p_def])
        ca << schema_info[0]
      end
    end
  end # class MPBase

  #
  # The Init method of OML4R
  # Ruby applications should call this method to initialise the OML4R module
  # This method will parse the command line argument of the calling application
  # to extract the OML specific parameters, it will also do the parsing for the
  # remaining application-specific parameters.
  # It will then connect to the OML server (if requested on the command line), and
  # send the initial instruction to setup the database and the tables for each MPs.
  #
  # param argv = the Array of command line arguments from the calling Ruby application
  # param opts
  # opts [String]  :domain
  # opts [String]  :nodeID
  # opts [String]  :appName
  # opts [Integer] :protocol
  # opts [Proc] :afterParse
  # param block = a block which defines the additional application-specific arguments
  #
  def self.init(argv, opts = {}, &block)
    OML4R.logger.info "OML4R Client #{VERSION} [OMSPv#{opts[:protocol] || DEF_PROTOCOL}; Ruby #{RUBY_VERSION}] #{COPYRIGHT}"
    if d = (ENV['OML_EXP_ID'] || opts[:expID])
      # NOTE: It is still too early to complain about that. We need to be sure
      # of the nomenclature before making user-visible changes.
      OML4R.logger.warn "opts[:expID] and ENV['OML_EXP_ID'] are getting deprecated; please use opts[:domain] or ENV['OML_DOMAIN']  instead"
      opts[:domain] ||= d
    end
    opts[:domain] = ENV['OML_DOMAIN'] || opts[:domain]

    # TODO: Same as above; here, though, :id might actually be the way to go; or
    # perhaps instId?
    #if opts[:id]
    #  raise 'OML4R: :id is not a valid option. Do you mean :nodeID?'
    #end
    opts[:nodeID]  = ENV['OML_NAME'] || opts[:nodeID]  ||  opts[:id] || ENV['OML_ID']
    #
    # XXX: Same again; also, this is the responsibility of the developer, not the user
    #if opts[:app]
    #  raise 'OML4R: :app is not a valid option. Do you mean :appName?'
    #end
    opts[:appName] ||= opts[:app]
    @@appName = opts[:appName]
    opts[:protocol] ||= DEF_PROTOCOL

    if  ENV['OML_URL'] || opts[:omlURL] || opts[:url]
      raise MissingArgumentException.new 'neither OML_URL, :omlURL nor :url are valid. Do you mean OML_COLLECT or :omlCollect?'
    end
    if ENV['OML_SERVER'] || opts[:omlServer]
        OML4R.logger.warn "opts[:omlServer] and ENV['OML_SERVER'] are getting deprecated; please use opts[:collect] or ENV['OML_COLLECT'] instead"
    end
    opts[:omlCollectUri] = ENV['OML_COLLECT'] || ENV['OML_SERVER'] || opts[:collect] || opts[:omlServer]
    noop = opts[:noop] || false
    omlConfigFile = nil

    if argv
      OML4R.logger.debug "ARGV: #{argv.inspect}"
      # Create a new Parser for the command line
      op = OptionParser.new
      # Include the definition of application's specific arguments
      yield(op) if block
      # Include the definition of OML specific arguments
      op.on("--oml-id id", "Name to identify this app instance [#{opts[:nodeID] || 'undefined'}]") { |name| opts[:nodeID] = name }
      op.on("--oml-domain domain", "Name of experimental domain [#{opts[:domain] || 'undefined'}] *EXPERIMENTAL*") { |name| opts[:domain] = name }
      op.on("--oml-collect uri", "URI of server to send measurements to") { |u|  opts[:omlCollectUri] = u }
      op.on("--oml-protocol p", "Protocol number [#{OML4R::DEF_PROTOCOL}]") { |l| opts[:protocol] = l.to_i }
      op.on("--oml-log-level l", "Log level used (info: 0 .. debug: 1)") { |l| OML4R.logger.level = 1 - l.to_i }
      op.on("--oml-noop", "Do not collect measurements") { OML4R.logger.info "OML reporting disabled from command line"
							   return  }
      op.on("--oml-config file", "File holding OML configuration parameters") { |f| omlConfigFile = f }
      op.on("--oml-exp-id domain", "Obsolescent equivalent to --oml-domain domain") { |name|
        opts[:domain] = name
        OML4R.logger.warn "Option --oml-exp-id is getting deprecated; please use '--oml-domain #{domain}' instead"
      }
      op.on("--oml-file localPath", "Obsolescent equivalent to --oml-collect file:localPath") { |name|
        opts[:omlCollectUri] = "file:#{name}"
        OML4R.logger.warn "Option --oml-file is getting deprecated; please use '--oml-collect #{opts[:omlCollectUri]}' instead"
      }
      op.on("--oml-server uri", "Obsolescent equivalent to --oml-collect uri") {|u|
        opts[:omlCollectUri] = "#{u}"
        OML4R.logger.warn "Option --oml-server is getting deprecated; please use '--oml-collect #{opts[:omlCollectUri]}' instead"
      }
      op.on_tail("--oml-help", "Show this message") { $stderr.puts op }
      # XXX: This should be set by the application writer, not the command line
      #op.on("--oml-appid APPID", "Application ID for OML [#{appName || 'undefined'}] *EXPERIMENTAL*") { |name| appID = name }
      unless opts[:appName]
	raise MissingArgumentException.new 'OML4R: Missing :appName in application code!'
      end

      # Now parse the command line
      rest = op.parse(argv)
      if opts[:afterParse]
        # give the app a chance to fix missing parameters
        opts[:afterParse].call(opts)
      end
      
      # Parameters in OML config file takes precedence
      unless omlConfigFile.nil? 
	f = File.open(omlConfigFile, 'r')
	f.each_line do |l|
	  d = l[/.*experiment=["']([^"']*)/,1]
	  opts[:domain] = d if d
	  d = l[/.*domain=["']([^"']*)/,1]
	  opts[:domain] = d if d
	  i = l[/.*id=["']([^"']*)/,1]
	  opts[:nodeID] = i if i
	  u = l[/.*url=["']([^"']*)/,1]
	  opts[:omlCollectUri] = u if u
	end
	f.close
      end
    end
 
    unless opts[:nodeID]
      begin
        # Create a default nodeID by concatenating the local hostname with the process ID
        hostname = nil
        begin
          #hostname = Socket.gethostbyname(Socket.gethostname)[0]
          hostname = Socket.gethostname
        rescue Exception
          begin
            hostname = (`hostname`).chop
          rescue Exception; end
        end
        if hostname
          opts[:nodeID] = "#{hostname}-#{Process.pid}"
        end
      end
      unless opts[:nodeID]
        raise MissingArgumentException.new 'OML4R: Missing values for parameter :nodeID (--oml-id, OML_ID)'
      end
    end
    unless opts[:domain]
      raise MissingArgumentException.new 'OML4R: Missing values for parameter :domain (--oml-domain, OML_DOMAIN)!'
    end

    # Set a default collection URI if nothing has been specified
    opts[:omlCollectUri] ||= "file:#{opts[:appName]}_#{opts[:nodeID]}_#{opts[:domain]}_#{Time.now.strftime("%Y-%m-%dt%H.%M.%S%z")}"
    opts[:omlCollectUri] = qualify_uri(opts[:omlCollectUri])
    OML4R.logger.info "Collection URI is #{opts[:omlCollectUri]}"

    create_channel(:default, opts[:omlCollectUri]) if opts[:omlCollectUri]

    # Handle the defined Measurement Points
    startTime = Time.now
    Channel.init_all(opts[:domain], opts[:nodeID], opts[:appName], startTime, opts[:protocol])
    rest || []
  end

  # Parse an underspecified URI into a fully-qualified one
  # URIs are resolved as follows; the default is [tcp:]host[:port].
  #  hostname		-> tcp:hostname:3003
  #  hostname:3004	-> tcp:hostname:3004
  #  tcp:hostname	-> tcp:hostname:3003
  #  tcp:hostname:3004	-> tcp:hostname:3004
  #  file:/P/A/T/H	-> file:/P/A/T/H
  #
  # @param uri [String] a potentially under-qualified collection URI
  #
  # @return [String] afully-qualified collection URI equivalent to uri
  #
  # @raise [OML4RException] in case of a parsing error
  #
  def self.qualify_uri(uri)
    curi = uri.split(':')

    # Defaults
    scheme = 'tcp'
    port = DEF_SERVER_PORT

    if curi.length == 1
      host = curi[0]

    elsif curi.length == 2
      if curi[0] == 'tcp'
        scheme, host = curi

      elsif  curi[0] == 'file'
        scheme, host = curi
        port = nil

      else
        host, port = curi
      end

    elsif curi.length >= 3
      if curi.length > 3
        OML4R.logger.warn "Parsing URI '#{uri}' as a triplet, ignoring later components"
      end
      scheme, host, port = curi

    else
      raise OML4RException.new "OML4R: Unable to parse URI '#{url}"
    end
    "#{scheme}:#{host}#{":#{port}" if port}"
  end
  
  def self.create_channel(name, url)
    Channel.create(name, url)
  end

  #
  # Close the OML collection. This will block until all outstanding data have been sent out.
  #
  def self.close()
    Channel.close_all
  end

  # Generate a random GUID
  #
  # @return [BigNum] An integer GUID.
  def self.generate_guid()
    SecureRandom.random_number(2**64)
  end


  #
  # Measurement Channel
  #
  class Channel
    @@channels = {}
    @@default_domain = nil

    def self.create(name, url, domain = :default)
      key = "#{name}:#{domain}"
      if channel = @@channels[key]
        if url != channel.url
          raise OML4RException.new "OML4R: Channel '#{name}' already defined with different url"
        end
        return channel
      end
      return self._create(key, domain, url)
    end

    def self._create(key, domain, url)
      @@channels[key] = self.new(url, domain)
    end

    # Parse the given fully-qualified collection URI, and return a suitably connected objet
    #
    # Supported URIs are
    #  tcp:host:port
    #  file:/P/A/T/H
    #
    # @param fquri [String] a fully qualified collection URI
    # @return [IO] an object suitably connected to the required URL
    #
    # @raise [OML4RException] in case of an unknown scheme
    #
    def self._connect(fquri)
      scheme, host, port = fquri.split(':')
      out = case scheme
            when 'tcp'
              out = TCPSocket.new(host, port)
            when 'file'
              # host is really a filename here
              out = (host == '-' ? $stdout : File.open(host, "w+"))
            else
              raise OML4RException.new "OML4R: Unknown scheme '#{scheme}"
            end
      out
    end

    def self.[](name = :default, domain = :default)
      key = "#{name}:#{domain}"
      unless (@@channels.key?(key))
        # If domain != :default and we have one for :default, create a new one
        if (domain != :default)
          if (dc = @@channels["#{name}:default"])
            return self._create(key, domain, dc.url)
          end
        end
        raise OML4RException.new "OML4R: Unknown channel '#{name}'"
      end
      @@channels[key]
    end

    def self.init_all(domain, nodeID, appName, startTime, protocol)
      @@default_domain = domain
      @@nodeID = nodeID
      @@appName = appName
      @@startTime = startTime
      @@protocol = protocol

      MPBase.__freeze__(appName, startTime)

      # send channel header
      @@channels.values.each { |c| c.init(nodeID, appName, startTime, protocol) }

      # send schema definitions
      MPBase.each_mp do |klass, defs|
        klass.__print_meta__(appName)
      end

      MPBase.__useOML__()
    end

    def self.close_all()
      @@channels.values.each { |c| c.close }
      @@channels = {}
      MPBase.__unfreeze__()
    end

    attr_reader :url

    def url=(url)
      return if @url == url
      if @out
        raise "Can't change channel's URL when it is already connected"
      end
      @url = url
    end

    def build_schema(mp_name, add_prefix, pdefs)
      @index += 1
      line = [@index, (!@@default_domain.nil? && add_prefix)? "#{@@default_domain}_#{mp_name}" : mp_name]

      pdefs.each do |d|
      	if !d[:relation].nil?
        	line << "#{d[:name]}:#{d[:type]}:#{d[:relation]}"
        else
        	line << "#{d[:name]}:#{d[:type]}"
        end
      end
      msg = line.join(' ')
      @schemas << msg
      [@index, msg]
    end

    def send(msg)
      @queue.push msg
    end
 
    def send_schema_update(msg)
      @header_sent = true
      @queue.push msg
    end

    def init(nodeID, appName, startTime, protocol)
      @nodeID, @appName, @startTime, @protocol = nodeID, appName, startTime, protocol
      @out = _connect(@url)
    end

    def close()
      @queue.push nil  # indicate end of work
      @runner.join()
    end

    protected
    def initialize(url, domain)
      @domain = domain
      @url = url
      @index = -1
      @schemas = []
      @header_sent = false
      @queue = Queue.new
      start_runner
    end

    def _connect(url)
      if url.start_with? 'file:'
        proto, fname = url.split(':')
        out = (fname == '-' ? $stdout : File.open(fname, "w+"))
      elsif url.start_with? 'tcp:'
        #tcp:norbit.npc.nicta.com.au:3003
        proto, host, port = url.split(':')
        port ||= DEF_SERVER_PORT
        out = TCPSocket.new(host, port)
      else
        raise OML4RException.new "OML4R: Unknown transport in server url '#{url}'"
      end
      @out = out
    end


    def _send_protocol_header(stream)
      header = []
      header << "protocol: #{@protocol}"
      header << "content: text"
      d = (@domain == :default) ? @@default_domain : @domain
      raise MissingArgumentException.new "Missing domain name" unless d
      case @protocol || OML4R::DEF_PROTOCOL
      when 4
        header << "domain: #{d}"
        header << "start-time: #{@startTime.tv_sec}"
        header << "sender-id: #{@nodeID}"
        header << "app-name: #{@appName}"
        @schemas.each do |s|
          header << "schema: #{s}"
        end
        header << ""
      else
        raise OML4RException.new "Unsupported protocol #{@protocol}"
      end
      stream.puts header
    end

    def start_runner
      @runner = Thread.new do
        active = true
        begin
          while (active)
            msg = @queue.pop
            active = !msg.nil?
            if !@queue.empty?
              ma = [msg]
              while !@queue.empty?
                msg = @queue.pop
                if (active = !msg.nil?)
                  ma << msg
                end
              end
              msg = ma.join("\n")
            end
            #$stderr.puts ">>>>>>#{@domain}: <#{msg}>"
            unless msg.nil?
              _send msg
            end
          end
          @out.close unless @out == $stdout
          @out = nil
        rescue Exception => ex
          OML4R.logger.warn "Exception while sending message to channel '#{@url}' (#{ex})"
        end
        OML4R.logger.info "Channel #{url} closed"
      end
    end

    def _send(msg)
      begin
        unless @header_sent
          _send_protocol_header(@out)
          @header_sent = true
        end
        @out.puts msg
        @out.flush

      rescue Errno::EPIPE
        # Trying to reconnect
        OML4R.logger.info "Trying to reconnect to '#{@url}'"
        loop do
          sleep 5
          begin
            @out = _connect(@url)
            @header_sent = false
            OML4R.logger.info "Reconnected to '#{@url}'"
            return _send(msg)
          rescue Errno::ECONNREFUSED => ex
            OML4R.logger.warn "Exception while reconnect '#{@url}' (#{ex.class})"
          end
          #Errno::ECONNREFUSED
        end
      end
    end

  end # Channel


  # Hard-code "schema0" measurement point
  class ExperimentMetadata < MPBase
    name :_experiment_metadata, :add_prefix => false
    param :subject, :type => :string
    param :key, :type => :string
    param :value, :type => :string    
  end


  require 'logger'

  class Logger < ::Logger
    def format_message(severity, time, progname, message)
      "%s\t%s\n" % [severity, message]
    end
  end

  @@logger = Logger.new(STDERR)
  @@logger.level = ::Logger::INFO

  def self.logger
    @@logger
  end

end # module OML4R

# vim: sw=2

