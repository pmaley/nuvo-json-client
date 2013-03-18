#!/usr/bin/env ruby

require 'json'
require 'socket'

host = ARGV[0] ? ARGV[0] : "192.168.1.101"
port = ARGV[1] ? ARGV[1] : 4747

puts "Connecting to #{host}, port #{port}."

@request_num = 0
@channels = {}
@requests = []
@socket = TCPSocket.open(host, port)

REQ_STABLE_AV = { "url" => "/stable/av/", "method" => "browse", "params" => { "count" => -1 } }
REQ_STABLE_AVS = { "url" => "/stable/avs/", "method"=> "browse", "params" => { "count" => -1 } }
REQ_STABLE_MUSIC = { "url" => "/stable/music/", "method" => "browse", "params" => { "count" => -1 } }

def send_request(request)
  request["id"] = "req-#{@request_num}"
  @requests << "req-#{@request_num}"
  puts request
  puts @requests.inspect
  @request_num += 1
  @socket.write "#{request.to_json}\n"
  return @request_num-1
end

def childInserted(channel,mesg)
  puts "INSIDE CHILD INSERTED"
  puts "Insert at index=#{mesg["index"]}"
  puts @channels[channel]["children"].length
  @channels[channel]["children"].insert mesg["index"], mesg["item"]
  puts @channels[channel]["children"].length
end

def childRemoved(channel,mesg)
  puts "INSIDE CHILD REMOVED"
  puts "Remove at index=#{mesg["index"]}"
  puts @channels[channel]["children"].length
  @channels[channel]["children"].delete_at mesg["index"]
  puts @channels[channel]["children"].length
end

def childValueChanged(channel,mesg)
  puts "INSIDE CHILD VALUE CHANGED"
end

def childItemChanged(channel,mesg)
  puts "INSIDE CHILD ITEM CHANGED"
end

def parse_closed_message(mesg)
  puts "CLOSE MESSAGE"
end

def parse_reply_message(mesg)
  @requests.delete(mesg["id"])
  puts @requests.inspect
  @channels[mesg["channel"]] = mesg["result"]
end

def parse_event_message(mesg)
  mesg["event"].each do |event|
    puts event["type"]
    method = "#{event["type"]}"
    send "#{event["type"]}", mesg["channel"], event
  end
end

def process_message(mesg)
  puts "*********************************************************"
  puts "#{mesg["type"]}: #{mesg["id"]}: #{mesg["channel"]}"

  method = "parse_#{mesg["type"]}_message"
  send method, mesg
end

av_req = send_request(REQ_STABLE_AV)
avs_req = send_request(REQ_STABLE_AVS)
music_req = send_request(REQ_STABLE_MUSIC)

stdin_buffer = ""
socket_buffer = ""

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


