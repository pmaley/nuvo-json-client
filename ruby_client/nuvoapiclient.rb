#!/usr/bin/env ruby

require 'json'
require 'socket'
require 'logger'
require 'yaml'

host = ARGV[0] ? ARGV[0] : "192.168.1.101"
port = ARGV[1] ? ARGV[1] : 4747

@logger = Logger.new(STDOUT)
@logger.info "Connecting to #{host}, port #{port}."

yml = File.read('config.yml')
config = YAML.load(yml)
@logger.info config.inspect

@request_num = 0
@channels = {}
@requests = []
@socket = TCPSocket.open(host, port)

REQ_STABLE_AV = { "url" => "/stable/av/", "method" => "browse", "params" => { "count" => -1 } }
REQ_STABLE_AVS = { "url" => "/stable/avs/", "method"=> "browse", "params" => { "count" => -1 } }
REQ_STABLE_MUSIC = { "url" => "/stable/music/", "method" => "browse", "params" => { "count" => -1 } }


########################
# SEND METHODS
########################
def send_request(request)
  request["id"] = @request_num
  @requests << @request_num
  @logger.info request
  @request_num += 1
  @socket.write "#{request.to_json}\n"
  return @request_num-1
end

def close_channel(channel)
  request = {"method" => "cancel", "params" => { "channels" => ["#{channel}"] } }
  send_request(request)
end

def load_av

end

########################
# RECEIVE METHODS
########################
def childInserted(channel,mesg)
  index = mesg["index"] ? mesg["index"] : 0
  @channels[channel]["children"].insert index, mesg["item"]
  @logger.info "ID: #{@channels[channel]["children"][index]["id"]}"
end

def childRemoved(channel,mesg)
  index = mesg["index"] ? mesg["index"] : 0
  @logger.info "ID: #{@channels[channel]["children"][index]["id"]}"
  @channels[channel]["children"].delete_at mesg["index"]
end

def childValueChanged(channel,mesg)
  index = mesg["index"] ? mesg["index"] : 0
  @logger.info "ID: #{@channels[channel]["children"][index]["id"]}"
  @logger.info @channels[channel]["children"][index]["value"].inspect
  @channels[channel]["children"][index]["value"] = mesg["value"]
  @logger.info @channels[channel]["children"][index]["value"].inspect
end

def childItemChanged(channel,mesg)
  index = mesg["index"] ? mesg["index"] : 0
  @logger.info "ID: #{@channels[channel]["children"][index]["id"]}"
  @channels[channel]["children"][index] = mesg["item"]
end

def parse_closed_message(mesg)
  @channels.delete(mesg["channel"])
end

def parse_reply_message(mesg)
  @requests.delete(mesg["id"])
  @channels[mesg["channel"]] = mesg["result"]
end

def parse_event_message(mesg)
  mesg["event"].each do |event|
    @logger.info "--------------------"
    @logger.info event["type"]
    method = "#{event["type"]}"
    send "#{event["type"]}", mesg["channel"], event
  end
end

def process_message(mesg)
  @logger.info "*********************************************************"
  @logger.info "#{mesg["type"]}: #{mesg["id"]}: #{mesg["channel"]}"
  @logger.info mesg.keys
  method = "parse_#{mesg["type"]}_message"
  send method, mesg
end


stdin_buffer = ""
socket_buffer = ""
thread1 = Thread.new{
  loop do
    if select( [$stdin], nil, nil, 0 )
      stdin_buffer +=  $stdin.read_nonblock(1024)
      @socket.write(stdin_buffer)
      stdin_buffer = ""
    end

    if select( [@socket], nil, nil, 0 )
      socket_buffer += @socket.read_nonblock(1024)
      a = socket_buffer.split("\n")
      socket_buffer = ""
      a.each_with_index do |mesg, i|
        begin
          b = JSON.parse(mesg)
          process_message(b)
        rescue
          if mesg != "Hello"
            socket_buffer = a.last 
          end
        end
      end
    end
  end
}


av_req = send_request(REQ_STABLE_AV)
avs_req = send_request(REQ_STABLE_AVS)
music_req = send_request(REQ_STABLE_MUSIC)

#sleep 10
#close_channel("ch2")

thread1.join


