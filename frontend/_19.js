// [_19.js] class _933 = JS 版 CBinaryBuffer —— 与固件 main/BinaryBuffer.cpp 逐语义同源:
//   _935()=干跑量尺寸(对应固件 +0 flag 字段) / _936()=按尺寸分配缓冲真写一遍(两遍构建);
//   _1082 等 set* 不传 endian 参数 => DataView 默认大端, 坐实协议大端序。
//   _1088=追加字符串(i32 长度前缀+ASCII字节), _1105=读字符串, _1104=读 UTF32 字符串(每字符 u32)
class _933
{
	_1077=false;
	position=0;
	_939=null;
	_1078=null;

	_1079(x)
	{
		if(!this._1077)
			this._1078.setInt8(this.position,x);
		this.position++;
	}
	_1080(x)
	{
		if (!this._1077)
			this._1078.setInt16(this.position, x);
		this.position += 2;
	}
	_1081(x)
	{
		if (!this._1077)
			this._1078.setUint16(this.position, x);
		this.position+=2;
	}
	_1082(x)
	{
		if (!this._1077)
			this._1078.setUint32(this.position, x);
		this.position += 4;
	}
	_1083(x)
	{
		let _1084;
		if(x)
			_1084=1;
		else
			_1084=0;
		this._1079(_1084);
	}
	_1082(x)
	{
		if (!this._1077)
			this._1078.setUint32(this.position, x);
		this.position += 4;
	}
	_1085(x)
	{
		if(!this._1077)
			this._1078.setInt32(this.position,x);
		this.position+=4;
	}
	_1086(x)
	{
		if(!this._1077)
			this._1078.setBigInt64(this.position,BigInt(x));
		this.position+=8;
	}
	_1087(x)
	{
		if(!this._1077)
			this._1078.setFloat64(this.position,x,true)
		this.position+=8;
	}
	_937(x)
	{
		this._1085(x.length);
		for(let _866=0;_866<x.length;_866++)
			this._1085(x.charCodeAt(_866));
	}
	_1088(x)
	{
		this._1085(x.length);
		for (let _866 = 0; _866 < x.length; _866++)
			this._1079(x.charCodeAt(_866));
	}
	_1089(x)
	{
		if (x.position != 0)
			console.assert(0);
		if(!this._1077)
		{
			let _1090=new Uint8Array(this._939);
			let _1091=new Uint8Array(x._939);
			_1090.set(_1091,this.position);
		}
		this.position+=x._939.byteLength;
	}
	_1092(x)
	{
		let _1093=new _933();
		_1093._1094(x);
		this._1089(_1093);
	}
	_971(_1095)
	{
		this._1085(_1095.length);
		for (let _866 = 0; _866 < _1095.length; _866++)
			this._1087(_1095[_866]);
	}

	_1096()
	{
		let _865;
		_865=this._1078.getInt8(this.position);
		this.position++;
		return _865;
	}
	_8130()
	{
		let _865;
		_865 = this._1078.getUint8(this.position);
		this.position++;
		return _865;
	}
	_1097(x)
	{
		let _865;
		_865 = this._1078.getInt16(this.position);
		this.position += 2;
		return _865;
	}
	_1098(x)
	{
		let _865;
		_865 = this._1078.getUint16(this.position);
		this.position += 2;
		return _865;
	}
	_1099()
	{
		let x;
		x=this._1096();
		if(x)
			return true;
		return false;
	}
	_1100()
	{
		let _865;
		_865=this._1078.getInt32(this.position);
		this.position+=4;
		return _865;
	}
	_1101()
	{
		let _865;
		_865 = this._1078.getUint32(this.position);
		this.position += 4;
		return _865;
	}
	_1102()
	{
		let _865;
		_865=this._1078.getBigInt64(this.position);
		this.position+=8;
		return _865;
	}
	_1103()
	{
		let _865;
		_865=this._1078.getFloat64(this.position,true)
		this.position+=8;
		return _865;
	}
	_8131()
	{
		let _865;
		_865 = this._1078._8132(this.position, true)
		this.position += 4;
		return _865;
	}
	_1104()
	{
		let _865;
		_865="";
		let length;
		length=this._1100();
		for(let _866=0;_866<length;_866++)
			_865+=String.fromCharCode(this._1100());
		return _865;
	}
	_1105()
	{
		let _865;
		_865 = "";
		let length;
		length = this._1100();
		for (let _866 = 0; _866 < length; _866++)
			_865 += String.fromCharCode(this._1096());
		return _865;
	}
	_1106(_1107,length)
	{
		console.assert(_1107.position==0);
		_1107._939=this._939.slice(this.position,this.position+length-1+1);
		_1107._1078=new DataView(_1107._939);
		this.position+=length;
	}
	_1108(length)
	{
		let _865;
		_865=this._939.slice(this.position,this.position+length);
		this.position+=length;
		return _865;
	}
	_1109(_1095)
	{
		_1095.length = this._1100();
		for (let _866 = 0; _866 < _1095.length; _866++)
			_1095[_866] = this._1103();
	}

	_935()
	{
		this.position=0;
		this._1077=true;
	}
	_936()
	{
		this._939=new ArrayBuffer(this.position);
		this.position=0;
		this._1077=false;
		this._1078=new DataView(this._939);
	}
	_1110(length)
	{
		this.position=length;
		this._936();
	}
	_1094(_1111)
	{
		this.position=0;
		this._939=_1111;
		this._1078=new DataView(this._939);
	}
	_1112(_1113)
	{
		let _987=new Blob([this._939],{type: 'application/bin'});
		let _1114=document.createElement('a');
		_1114.href=window.URL.createObjectURL(_987);
		_1114.download=_1113;
		_1114.click();
		_1114.remove();
	}

	_1115(x)
	{
		if(this._939.byteLength!=x._939.byteLength)
			return false;
		for(let _866=0;_866<this._939.byteLength;_866++)
			if(this._939[_866]!=x._939[_866])
				return false;
		return true;
	}

	_1116()
	{
		return this._939.byteLength;
	}

	_1025()
	{
		this._1077=false;
		this.position=0;
		this._939=null;
		this._1078=null;
	}
	_8133()
	{
		return this._939 != null;
	}

	_1117()
	{
		if (!this._939)
			return "";
		const _1118 = new Uint8Array(this._939);
		let result = '';
		for (let _1119 = 0; _1119 < _1118.length; _1119 += 16) 
		{
			const _1120 = _1119.toString(16).padStart(8, '0');
			const slice = _1118.slice(_1119, _1119 + 16);
			const _1121 = Array.from(slice)
				.map(_842 => _842.toString(16).padStart(2, '0'))
				.join(' ');
			const padding = '   '.repeat(16 - slice.length);
			const _1122 = Array.from(slice)
				.map(_842 => (_842 >= 32 && _842 <= 126) ? String.fromCharCode(_842) : '.')
				.join('');
			result += _1120+'  ' + _1121 + padding+'  ' + _1122 + '\n';
		}
		return result.trim();		
	}
	static _1123(_1107, _1111, _1124)
	{
		if (!_1107._1077)
		{
			let _1125 = new Uint8Array(_1107._939);
			let _1126 = new Uint8Array(_1111._939);
			let slice = _1126.subarray(_1111.position, _1111.position + _1124);
			_1125.set(slice, _1107.position);
		}
		_1107.position += _1124;
		_1111.position += _1124;
	}
};

