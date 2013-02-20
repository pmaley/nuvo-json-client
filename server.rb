#!/usr/bin/env ruby

require 'socket'
require 'json'

response= <<"EOF";                # same as above
  { "type" : "reply", "id" : "req-1", "channel" : "ch-2", "result" : {
  "item" : {
    "url" : "/av01/",
    "type" : "container",
    "title" : "Kitchen" 
  },
  "count" : 14,
  "children" : [
    { "id" : "state", "type" : "value", "value" : { "avState" : "playing" } },
    { "id" : "status", "type" : "value", "value" : { "string" : "" } },
    { "id" : "mode", "url" : "/av01/mode", "type" : "value", "value" : { "avMode" : "repeatAll" }, "modifiable" : true },
    { "id" : "shuffle", "url" : "/av01/shuffle", "type" : "value", "value" : { "bool" : false }, "modifiable" : true },
    { "id" : "item",  "url" : "/rhapsody/album?id=34", "type" : "container", "title" : "The Wall - Pink Floyd", "artist" : "Pink Floyd", "album" : "The Wall" },
    { "id" : "trackIndex", "url" : "/av01/trackIndex", "type" : "value", "value" : { "int" : 0 }, "modifiable" : true, "maxInt" : 15 },
    { "id" : "trackItem", "media" : true, "title" : "Track 1", "artist" : "Pink Floyd", "album" : "The Wall", "resource" : "http://rhasody.com/tr00", "creator" : "rhapsody" },
    { "id" : "pause", "url" : "/av01/pause", "type" : "action" },
    { "id" : "next", "url" : "/av01/next", "type" : "action" },
    { "id" : "time", "url" : "/av01/time", "type" : "value", "value" : { "double" : 3.0 }, "modifiable" : true, "maxDouble" : 286.3 },
    { "id" : "bufferLevel", "url" : "/av01/bufferLevel", "type" : "value", "value" : { "double" : 100.0 }, "maxDouble" : 100, "maxDoubleInclusive" : true },
    { "id" : "info", "title" : "Track 1", "description" : "The Wall", "longDescription" : "Pink Floyd" },
    { "id" : "volume", "url" : "/av01/volume", "type" : "value", "value" : { "int" : 55 }, "modifiable" : true, "maxInt" : 101 },
    { "id" : "mute", "url" : "/av01/mute", "type" : "value", "value" : { "bool" : false }, "modifiable" : true }
  ]
} }
EOF

server = TCPServer.new 2000 # Server bind to port 2000
loop do
  client = server.accept    # Wait for a client to connect
  
  # initial handshake
  client.puts "Protocol: Tesla JSON 1.0"
  client.gets
  client.puts "Channel-Timeout: 300"
  #client.puts ""
  # end of handshake

  while !client.closed?
    client.gets
    client.puts JSON.parse(response).to_json
  end

  puts "Closing client"
  client.close
end
