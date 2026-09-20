// [_27.js] class _1170 = BLE 协议层 —— 对应固件 main/JsInterface.cpp + driver/BluetoothManager.cpp
//   包魔数 _8137 = 0x4E8A2F5B(大端); 上行 25 命令 / 下行 16 响应(见 _1191 switch);
//   MTU 硬上限 509(_8135), OTA 分块负载 455(_8136), PickupPluginSendToCpp ≤459。
//   上行包 = [u32 魔数][string 命令名][负载]; 下行包 = [string 命令名][u32 total][u32 pos][u32 len][data] 重组
class _1170
{
	
	static _8134 = 509;
	static _8135 = 509;
	static _8136 = 455;

	static _8137 = 0x4e8a2f5b;
	static _8138 = new _933();
	static _8139 = new _933();
	static _1191(data)
	{
		let buffer = new _933();
		buffer._1094(data);
		let _1193;
		_1193 = buffer._1105();
		let _8140, position, _8141;
		_8140 = buffer._1100();
		position = buffer._1100();
		_8141 = buffer._1100();
		if (position == 0)
			_1170._8138._1110(_8140);
		console.assert(_1170._8138.position == position);
		_933._1123(_1170._8138, buffer, _8141);
		if (_1193 == "RecordTemporarilyMonoData" || _1193 == "RecordTemporarilyStereoData")
			_690._1160(_1170._8138.position, _8140);
		else if (_1193 == "UserConfigure")
			_626._1160(_1170._8138.position, _8140);
		if (_1170._8138.position >= _8140)
		{
			_1170._8138.position = 0;
			switch (_1193)
			{
				case "RecordTemporarilyMonoData":
					_8142(_1170._8138, 1);
					break;
				case "RecordTemporarilyStereoData":
					_8142(_1170._8138, 2);
					break;
				case "UserConfigure":
					_8143(_1170._8138);
					if (_8144) { _8144(); _8144 = null; }
					break;
				case "ShowDebugString":
					_691.innerText = new TextDecoder('ascii').decode(_1170._8138._939);
					break;
				case "ShowCpuUsageString":
					_693.innerText = new TextDecoder('ascii').decode(_1170._8138._939);
					break;
				case "ShowMemroyUsageString":
					_695.innerText = new TextDecoder('ascii').decode(_1170._8138._939);
					break;
				case "FirmwareVersion":
					_8145(new TextDecoder('ascii').decode(_1170._8138._939));
					if (_8144){_8144();_8144 = null;}
					break;
				case "PickupPluginBaseAddress":
					_8146 = _1170._8138._1101();
					_8147 = _1170._8138._1101();
					if (_8144) { _8144(); _8144 = null; }
					break;
				case "PickupPluginHtml":
					{
						
						
						
						let _5948, _8148;
						_5948 = _1170._8138._1101();
						_8148 = _1170._8138._1101();
						if (_5948 == 0)
							_1170._8139._1110(_8148);
						_933._1123(_1170._8139, _1170._8138, _1170._8138._1116() - _1170._8138.position);
						if (_1170._8139.position >= _8148)
						{
							
							_8149(_1170._8139._939);
							_1170._8139._1025();
						}
					}
					break;
				case "BluetoothDriveSpeakState":
					_8150(_1170._8138);
					if (_8144) { _8144(); _8144 = null; }
					break;
				case "MidiDeviceState":
					_8151(_1170._8138);
					if (_8144) { _8144(); _8144 = null; }
					break;
				case "BluetoothDriveSpeakDiscovered":
					_8152(_1170._8138);
					break;
				case "CurrentOutput":
					_8153(new TextDecoder('ascii').decode(_1170._8138._939));
					break;
				case "UpdateFirmwareData_Success":
					_8154();
					_614.onclick();
					_623.style.display = "";
					break;
				case "UpdateFirmwareData_Failed":
					alert(L("固件升级失败"));
					_878._879(_682, true);
					_620.onclick();
					break;
				case "PickupPluginSetJs":
					if (_645 && _645.contentWindow._8155)
						_645.contentWindow._8155(_1170._8138._939);
					else
						console._8156(L("PickupPluginSetJs no PickupPluginOnCpp function found!"));
					break;
			}
			_1170._8138._1025();
		}
	}
	static async _8157(buffer)
	{
		console.assert(_8158);
		console.assert(buffer._1116() <= _1170._8135);
		await _8158.writeValue(buffer._939);
	}

	static async _8159(_1193)
	{
		let buffer = new _933();
		for (let _934 = 0; _934 < 2; _934++)
		{
			if (_934 == 0)
				buffer._935();
			else
				buffer._936();
			buffer._1082(_1170._8137);
			buffer._1088(_1193);
		}
		await this._8157(buffer);
	}
	static async _8160(_1193,_1702)
	{
		let buffer = new _933();
		for (let _934 = 0; _934 < 2; _934++)
		{
			if (_934 == 0)
				buffer._935();
			else
				buffer._936();
			buffer._1082(_1170._8137);
			buffer._1088(_1193);
			buffer._1088(_1702);
		}
		await this._8157(buffer);
	}
	static async _8161()
	{
		await this._8159("GetDebugString");
	}
	static async _8162()
	{
		await this._8159("GetCpuUsageString");
	}
	static async _8163()
	{
		await this._8159("GetMemoryUsageString");
	}
	static async _8164(_1111)
	{
		await this._8160("RecordTemporarily", _1111);
	}
	static async _8165(_1111)
	{
		await this._8160("GetHistoryRecord", _1111);
	}
	static async _8166()
	{
		await this._8159("GetFirmwareVersion");
	}
	static async _8167()
	{
		await this._8159("GetUserConfigure");
	}
	static async _8168()
	{
		await this._8159("GetBluetoothDriveSpeakState");
	}
	static async _8169()
	{
		await this._8159("GetMidiDeviceState");
	}
	static async _8170()
	{
		await this._8159("GetPickupPluginBaseAddress");
	}
	static async _8171()
	{
		await this._8159("GetPickupPluginHtml");
	}
	static async _8172(_1360)
	{
		await this._8160("SetOutput", _1360);
	}
	static async _8173()
	{
		await this._8159("SetAutoPitch");
	}
	static async _8174()
	{
		await this._8159("SetPickup");
	}
	static async _8175()
	{
		await this._8159("SetMidiDevice");
	}
	static async _8176()
	{
		await this._8159("BluetoothDriveSpeakerDisconnect");
	}
	static async _8177()
	{
		await this._8159("BluetoothDriveSpeakerSearch");
	}
	static async _8178()
	{
		await this._8159("DisconnectMidiDevice");
	}
	static async _8179(_8180,name)
	{
		const _8181 = new ArrayBuffer(6);
		{
			
			
			
			
			const _1880 = _8180.split(':').map(_1121 => parseInt(_1121, 16));
			const _8182 = new Uint8Array(_8181);
			_8182.set(_1880);
		}

		const _8183 = new ArrayBuffer(29+1);
		{
			const _8184 = new TextEncoder();
			const _8185 = _8184.encode(name);
			const _8182 = new Uint8Array(_8183);
			const _8186 = Math.min(_8185.length, 29+1);
			_8182.set(_8185.subarray(0, _8186), 0);
			_8182[29] = 0;
		}

		let buffer = new _933();
		for (let _934 = 0; _934 < 2; _934++)
		{
			if (_934 == 0)
				buffer._935();
			else
				buffer._936();
			buffer._1082(_1170._8137);
			buffer._1088("BluetoothDriveSpeakerConnect");
			buffer._1092(_8181);
			buffer._1092(_8183);
		}
		await this._8157(buffer);
	}
	static async _8187(_8188, _8189, _8190)
	{
		let buffer = new _933();
		for (let _934 = 0; _934 < 2; _934++)
		{
			if (_934 == 0)
				buffer._935();
			else
				buffer._936();
			buffer._1082(_1170._8137);
			buffer._1088("MountMotor");
			buffer._1088(_8188);
			buffer._1088(_8189);
			buffer._1085(_8190);
		}
		await this._8157(buffer);
	}
	static async _8191()
	{
		await this._8159("UpdateFirmwareDataCanceled");
	}
	static async _8192(_939)
	{
		
		
		
		const _8193 = _1170._8135 - 50;
		if (_939.length > _8193)
		{
			console.error(L("PickupPluginSendToCpp length limited to ") + _8193 + L(", because MCU memory limited, cannot concatenate large buffers"));
			return 0;
		}
		let buffer = new _933();
		for (let _934 = 0; _934 < 2; _934++)
		{
			if (_934 == 0)
				buffer._935();
			else
				buffer._936();
			buffer._1082(_1170._8137);
			buffer._1088("PickupPluginSendToCpp");
			buffer._1092(_939);
		}
		await this._8157(buffer);
	}
}

function _8142(_8194, _1883)
{
	let _1876;
	_1876 = _8194._1100();
	let _939;
	_939 = _8194._1108(_8194._1116() - _8194.position);
	let _1875 = new Int16Array(new Uint16Array(_8194._939).map(_8195 => _8195 - 0x8000).buffer);
	let _7787 = _878._1874(_1875, _1876, _1883);
	let _987 = new Blob([_7787], { "type": "audio/wav", });
	let _1114 = document.createElement("a");
	_1114.href = window.URL.createObjectURL(_987);
	if (_8196 == "input")
		_1114.download = P("_9748.wav");
	else
		_1114.download = P("_9749.wav");
	_8196 = null;
	_1114.click();
	_1114.remove();

	_690.style.display = "none";
	_8197._8198();
}
function _8143(_1093)
{
	_627.style.display = "";
	_627.disabled = false;
	_624.style.display = "none";
	_625.style.display = "none";

	function _8199(name)
	{
		_1093.position = 0;
		while (true)
		{
			let _2198;
			_2198 = _1093._1101();
			if (_2198 != 0xE45B901F)
				return false;
			let _8200;
			_8200 = _1093._1105();
			if (_8200 == name)
				return true;
			let size;
			size = _1093._1100();
			_1093.position += size;
		}
	}
	function _8201(name)
	{
		if (!_8199(name))
			return null;
		return _1093._1105();
	}
	function _8202(name)
	{
		if (!_8199(name))
			return null;
		let size;
		size = _1093._1100();
		console.assert(size == 4);
		return _1093._1100();
	}
	function _8203(name)
	{
		if (!_8199(name))
			return null;
		size = _1093._1100();
		console.assert(size == 1);
		return _1093._1099();
	}

	_8204();
	{
		let _8205;
		_8205 = _8201(F("module"));
		_8206(_8205);
	}
	{
		let _1360;
		_1360 = _8201(F("output"));
		if (_1360 === null)
			_1360 = F("line_out");
		_8153(_1360);
	}
}
function _8145(_8207)
{
	_680.innerText = _8207;
	if (_681.innerText == _8207 || (!_8208._8133()) )
		_878._879(_682, false); 
	else
		_878._879(_682, true); 

}
function _8150(_1093)
{
	
	
	
	let state;
	state = _1093._1105();

	switch (state)
	{
		case "not_initialzied":
			_658.src = P("_271.png"); 
			_665.innerHTML = "";
			_669.disabled = true;
			_660.style.display = "none";
			_664.style.display = "none";
			_667.disabled = false;
			_666.style.display = "none";
			_668.style.display = "none";
			break;
		case "free":
			if (_667._8209 || _669._8209)
			{
				_667._8209 = null;
				_669._8209 = null;
				_8197._8198();
			}
			_665.innerHTML = "";
			_669.disabled = true;
			_660.style.display = "none";
			_664.style.display = "";
			_667.disabled = false;
			_666.style.display = "none";
			_668.style.display = "none";
			break;
		case "searching_speaker":
			if (_667._8209)
			{
				_667._8209 = null;
				_8197._8198();
			}
			_665.innerHTML = "";
			_669.disabled = true;
			_660.style.display = "none";
			_664.style.display = "";
			_667.disabled = true;
			_666.style.display = "";
			_668.style.display = "none";
			break;
		case "connecting":
			{
				_665.innerHTML = "";
				_669.disabled = true;
				_660.style.display = "none";
				_664.style.display = "";
				_667.disabled = true;
				_666.style.display = "none";
				_668.style.display = "";
			}
			break;
		case "connected":
			{
				if (_667._8209 || _669._8209)
				{
					_667._8209 = null;
					_669._8209 = null;
					_8197._8198();
				}
				_665.innerHTML = "";
				_669.disabled = true;
				_660.style.display = "";
				_664.style.display = "none";
				_666.style.display = "none";
				_668.style.display = "none";
				let _8180;
				_8180 = _1093._1108(6);
				_8180 = new Uint8Array(_8180);
				_8180 = Array.from(_8180).map(_834 => _834.toString(16).padStart(2, '0').toUpperCase()).join(':');
				let name;
				name = _1093._1108(29 + 1);
				name = new Uint8Array(name);
				name = new TextDecoder('utf-8').decode(name.subarray(0, name.indexOf(0) !== -1 ? name.indexOf(0) : name.length));
				_661.innerText = name;
				_662.innerText = _8180;
			}
			break;
	}
}
function _8151(_1093)
{
	if (_651._8209)
	{
		_651._8209 = null;
		_8197._8198();
	}
	let _8210, _8211, name;
	_8210 = _1093._1099();
	_8211 = _1093._1099();
	name = _1093._1105();
	_8212(_8210, _8211, name);
}
function _8152(_1093)
{
	
	
	let _8180;
	_8180 = _1093._1108(6);
	_8180 = new Uint8Array(_8180);
	_8180 = Array.from(_8180).map(_834 => _834.toString(16).padStart(2, '0').toUpperCase()).join(':');
	let name;
	name = _1093._1108(29 + 1);
	name = new Uint8Array(name);
	name = new TextDecoder('utf-8').decode(name.subarray(0, name.indexOf(0) !== -1 ? name.indexOf(0) : name.length));

	let _8213;
	_8213 = A("_669._8377='") + _8180 + A("';");
	_8213 += A("_669._8378='") + name + A("';");
	_8213 += A("_669.disabled=false;");
	_8213 += A("_8374();");
	_8213 += A("this.style.backgroundColor='rgba(0, 0, 255, 0.2)';");
	let _8214;
	_8214 = A("_8324('") + _8180 + A("', '") + name + A("');");
	let _1147;
	_1147 = "";
	_1147 += "<div onclick=\"" + _8213 + "\" ondblclick=\"" + _8214 + "\">";
	_1147 += "<span>" + name + "</span>&nbsp&nbsp";
	_1147 += "<span style='opacity: 0.4;'>" + _8180 + "</span>";
	_1147 += "</div>\r\n";

	let _8215;
	_8215 = _665.getElementsByTagName("div");
	for (let _866 = 0; _866 < _8215.length; _866++)
	{
		let _8216;
		_8216 = _8215[_866];
		let _8217;
		_8217 = _8216.getElementsByTagName("span");
		let _8218,_8200;
		_8218 = _8217[1].innerText;
		_8200 = _8217[0].innerText;
		if (_8218 == _8180)
		{
			if (_8200.length < name.length)
			{
				_8216.innerHTML = "<span>" + name + "</span>&nbsp&nbsp<span style='opacity: 0.4;'>" + _8180 + "</span>";
				_8216.setAttribute('onclick', _8213);
				_8216.setAttribute('ondblclick', _8214);
				_8216.outerHTML = _1147;
			}
			return;
		}
	}
	_665.innerHTML += _1147;
}


async function _8192(_939)
{
	await _1170._8192(_939);
}



