defmodule Data do
  def get_bson do
    get_data |> Enum.map(&CBson.encode/1)
  end

  def get_data do
    [
      %{
        a:  -4.230845,
        b:  "hello",
        c:  %{x: -1, y: 2.2001},
        d:  [23, 45, 200],
        eeeeeeeee:  %Bson.Bin{ subtype: Bson.Bin.subtyx(:binary),
          bin:  <<200, 12, 240, 129, 100, 90, 56, 198, 34, 0, 0>>},
        f:  %Bson.Bin{ subtype: Bson.Bin.subtyx(:function),
          bin:  <<200, 12, 240, 129, 100, 90, 56, 198, 34, 0, 0>>},
        g:  %Bson.Bin{ subtype: Bson.Bin.subtyx(:uuid),
          bin:  <<49, 0, 0, 0, 4, 66, 83, 79, 78, 0, 38, 0, 0, 0,
          2, 48, 0, 8, 0, 0, 0, 97, 119, 101, 115, 111, 109,
          101, 0, 1, 49, 0, 51, 51, 51, 51, 51, 51, 20, 64,
          16, 50, 0, 194, 7, 0, 0, 0, 0>>},
        h:  %Bson.Bin{ subtype: Bson.Bin.subtyx(:md5),
          bin:  <<200, 12, 240, 129, 100, 90, 56, 198, 34, 0, 0>>},
        i:  %Bson.Bin{ subtype: Bson.Bin.subtyx(:user),
          bin:  <<49, 0, 0, 0, 4, 66, 83, 79, 78, 0, 38, 0, 0, 0, 2,
          48, 0, 8, 0, 0, 0, 97, 119, 101, 115, 111, 109, 101,
          0, 1, 49, 0, 51, 51, 51, 51, 51, 51, 20, 64, 16, 50,
          0, 194, 7, 0, 0, 0, 0>>},
        j:  %Bson.ObjectId{oid: <<82, 224, 229, 161, 0, 0, 2, 0, 3, 0, 0, 4>>},
        k1: false,
        k2: true,
        l:  Bson.UTC.from_now({1390, 470561, 277000}),
        m:  nil,
        q1: -2000444000,
        q2: -8000111000222001,
        r:  %Bson.Timestamp{ts: 2},
        t: Bson.ObjectId.from_string("52e0e5a10000020003000004")
      },
      %{
        "ip": "112.113.57.181",
        "data": %{
          "graphicsDeviceVersion": "OpenGL ES 2.0 V@6.0 AU@ (CL@3657481)",
          "npotSupport": "Full",
          "supportedRenderTargetCount": 1,
          "supportsVibration": true,
          "dataPath": "\/data\/app\/com.tencent.tmgp.ttlz-1.apk",
          "graphicsMemorySize": 251,
          "processorType": "ARMv7 VFPv3 NEON",
          "operator": "qqapp",
          "processorCount": 4,
          "supportsComputeShaders": false,
          "absoluteURL": "",
          "graphicsDeviceVendorID": 0,
          "genuine": true,
          "temporaryCachePath": "\/storage\/sdcard0\/Android\/data\/com.tencent.tmgp.ttlz\/cache",
          "targetFrameRate": -1,
          "systemMemorySize": 1797,
          "supportsGyroscope": true,
          "supportsStencil": 1,
          "platform": "Android",
          "supportsAccelerometer": true,
          "version": "1.0.70706",
          "deviceType": "Handheld",
          "supportsRenderToCubemap": true,
          "supportsShadows": true,
          "utcDateTime": 1410404000,
          "persistentDataPath": "\/storage\/sdcard0\/Android\/data\/com.tencent.tmgp.ttlz\/files",
          "description": "应用宝 2014-07-02",
          "graphicsShaderLevel": 30,
          "supportsInstancing": false,
          "streamingAssetsPath": "jar:file:\/\/\/data\/app\/com.tencent.tmgp.ttlz-1.apk!\/assets",
          "supportsRenderTextures": true,
          "systemLanguage": "Chinese",
          "maxTextureSize": 4096,
          "supportsVertexPrograms": true,
          "graphicsDeviceName": "Adreno (TM) 320",
          "deviceModel": "LGE LG-E988",
          "internetReachability": "ReachableViaLocalAreaNetwork",
          "graphicsDeviceVendor": "Qualcomm",
          "graphicsPixelFillrate": -1,
          "unityVersion": "4.5.1f3",
          "dateTime": 1410433000,
          "graphicsDeviceID": 0,
          "genuineCheckAvailable": false,
          "supportsImageEffects": true,
          "supportsLocationService": true,
          "supports3DTextures": false,
          "deviceUniqueIdentifier": "27bd20d0f89d1f7611bbc996ee0e0817",
          "operatingSystem": "Android OS 4.1.2 \/ API-16 (JZO54K\/E98810d.1377827792)"
       }
      },
      %{
        "ip": "112.113.57.181",
        "data": %{
          "graphicsDeviceVersion": "OpenGL ES 2.0 V@6.0 AU@ (CL@3657481)",
          "npotSupport": "Full",
          "supportedRenderTargetCount": 1,

          "subdata": %{
            "dateTime": 1410433000,
            "graphicsDeviceID": 0,
            "genuineCheckAvailable": false,
            "supportsImageEffects": true,
            "supportsLocationService": true,
            "subdata": %{
              "dateTime": 1410433000,
              "graphicsDeviceID": 0,
              "genuineCheckAvailable": false,
              "supportsImageEffects": true,
              "supportsLocationService": true,
            }
          }
        }
      },
      %{
        "_id"=> Bson.ObjectId.from_string("5625d0b75f6bcdac0556dcec"),
    "avatar"=> %{
        "create"=> nil,
        "id"=> "E575D708-8A61-9B3D-896F-4895809F63E5",
        "name"=> nil,
        "size"=> nil,
        "source"=> "MOMO",
        "type"=> nil
    },
    "bio"=> "找一个能相知相伴一直走下去的人真的好难。",
    "city"=> "360700",
    "contacts"=> [],
    "day"=> 7,
    "eula"=> false,
    "ext"=> %{
        "vip"=> 0
    },
    "fresh"=> true,
    "gender"=> 1,
    "loc"=> %{
        "coordinates"=> [ 
            114.936872, 
            25.855845
        ],
        "type"=> "Point"
    },
    "lock"=> true,
    "month"=> 3,
    "name"=> "     阳光如旧",
    "photos"=> [ 
        %{
            "create"=> nil,
            "id"=> "E575D708-8A61-9B3D-896F-4895809F63E5",
            "name"=> nil,
            "size"=> nil,
            "source"=> "MOMO",
            "type"=> nil
        }, 
        %{
            "create"=> nil,
            "id"=> "97BD960B-E4ED-FCDE-1F7D-629990C4A87C20151019",
            "name"=> nil,
            "size"=> nil,
            "source"=> "MOMO",
            "type"=> nil
        }, 
        %{
            "create"=> nil,
            "id"=> "15962FFD-60C7-9F72-2F65-5A4E561C75C820151019",
            "name"=> nil,
            "size"=> nil,
            "source"=> "MOMO",
            "type"=> nil
        }, 
        %{
            "create"=> nil,
            "id"=> "ED626DC0-7AAF-CD9C-B87C-AD1138C91D9620151019",
            "name"=> nil,
            "size"=> nil,
            "source"=> "MOMO",
            "type"=> nil
        }, 
        %{
            "create"=> nil,
            "id"=> "612A2B4B-484B-14D6-0710-BAEF532A674C20151019",
            "name"=> nil,
            "size"=> nil,
            "source"=> "MOMO",
            "type"=> nil
        }, 
        %{
            "create"=> nil,
            "id"=> "5C8DE3DA-72EC-1912-749C-D339C4BAAB5D20151019",
            "name"=> nil,
            "size"=> nil,
            "source"=> "MOMO",
            "type"=> nil
        }, 
        %{
            "create"=> nil,
            "id"=> "98FF1781-82D0-6138-347D-25BC22D3F5A120151019",
            "name"=> nil,
            "size"=> nil,
            "source"=> "MOMO",
            "type"=> nil
        }, 
        %{
            "create"=> nil,
            "id"=> "B35CDBF8-471D-0672-8F21-00CB2E40258820151019",
            "name"=> nil,
            "size"=> nil,
            "source"=> "MOMO",
            "type"=> nil
        }
    ],
    "update_mark"=> Bson.UTC.from_now(:erlang.timestamp),
    "year"=> 1996
}
    ]
  end
end

defmodule DecodBench do
  use Benchfella

  bench "decoder(cbson)", [bson: Data.get_bson()] do
    bson |> Enum.each(&Bson.decode/1)
  end
  
  bench "load_json(cbson)", [json: get_json()] do
    json |> Enum.each(&Bson.JsonExt.load/1)
  end

  defp get_json do
    Data.get_data |> Enum.map(&Bson.JsonExt.dump/1)
  end
end

defmodule EncodeBench do
  use Benchfella

  bench "encode(cbson)", [bson: get_bson()] do
    bson |> Enum.each(&Bson.encode/1)
  end

  bench "to_json(cbson)", [bson: get_bson()] do
    bson |> Enum.each(&Bson.JsonExt.dump/1)
  end

  bench "to_plain_json(cbson)", [bson: get_bson()] do
    bson |> Enum.each(&Bson.JsonExt.plain_dump/1)
  end

  bench "b64(elixir)", [data: Data.get_bson()] do
    data |> Enum.each(&Base.encode64/1)
  end
  
  bench "b64(cbson)", [data: Data.get_bson()] do
    data |> Enum.each(&CBson.nif_b64encode/1)
  end

  defp get_bson do
    Data.get_bson |> Enum.map(&CBson.decode/1)
  end
end

defmodule SplitBench do
  use Benchfella
  
  bench "peek_cstring(elixir)", [docs: doc()] do
    docs |> Enum.each(fn x ->
      peek_cstring(x, 4)
    end)
  end
  
  bench "peek_cstring(cbson)" , [docs: doc()] do
    docs |> Enum.each(fn x ->
      CBson.nif_split_by_char(x, 0, 4)
    end)
  end

  defp peek_cstring(buffer, drop) do
    {pos, _len} = :binary.match(buffer, "\0", [{:scope, {drop, byte_size(buffer) - drop}}])
    len = pos - drop
    <<_::size(drop)-binary, name::size(len)-binary, 0::size(8), rest::binary>> = buffer
    {name, rest}
  end

  defp doc do
    Data.get_bson() |> Enum.map(fn x -> 
      <<0, 0, 0, 0, "database.hello_world", 0, x::binary>>
    end)
  end
end

