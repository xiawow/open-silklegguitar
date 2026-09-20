// [_22.js] 复选框组件 _1143: 图标 _24.png(开)/_23.png(关) 切换, 带禁用置灰
function _1143(_1144,_1145,name,width,height,_1146)
{
	if(!width)
		width=37;
	if(!height)
		height=37;
	if(!_1146&&_1146!=0&&_1146!=false)
		_1146=true;
	var _1147;
	_1147="<span style='display:inline-block;text-align:center;"+"'>\r\n";
	_1147+="	<img src='_24.png' style='width:"+width+"px;height:"+height+"px;' />\r\n";
	_1147+="	<img src='_23.png' style='width:"+width+"px;height:"+height+"px;' />\r\n";
	if(name)
	{
		if(_1146)
			_1147+="<br/>\r\n";
		_1147+="<span>"+name+"</span>\r\n";
	}
	_1147+="</span>\r\n";
	_1144.innerHTML=_1147;
	_1144._1145 = _1145;
	{
		let _1148;
		_1148 = _1144.getElementsByTagName("span")[0];
		let _1149;
		_1149 = _1148.getElementsByTagName("img");
		_1144._24 = _1149[0];
		_1144._23 = _1149[1];
		_1144._1150 = _1148.getElementsByTagName("span")[0];
	}
	_1144._24._1144 = _1144;
	_1144._23._1144 = _1144;
	_1144._1151=function ()
	{
		this._1152=true;
		this._1153(true);
	}
	_1144._1154=function ()
	{
		this._1152=false;
		this._1153(true);
	}
	_1144._1153=function (_1155)
	{
		if(this._1152)
		{
			this._24.style.display="inline";
			this._23.style.display="none";
		}
		else
		{
			this._23.style.display="inline";
			this._24.style.display="none";
		}
		if(_1155)
		{
			if(this._1145!=null)
				this._1145(this._1152);
		}
	}
	_1144._1156=function (_1157)
	{
		this._1158=_1157;
		if(this._1158)
		{
			this._24.style.webkitFilter ="";
			this._23.style.webkitFilter ="";
			this._24.style.cursor="hand";
			this._23.style.cursor="hand";
			if(this._1150)
				this._1150.style.color="";
			this._1153(false);
			if (this._24._1159)
				this._24.onclick = this._24._1159;
			if (this._23._1159)
				this._23.onclick = this._23._1159;
		}
		else
		{
			this._24.style.webkitFilter ="grayscale(100%)";
			this._23.style.webkitFilter ="grayscale(100%)";
			this._24.style.cursor="";
			this._23.style.cursor="";
			if(this._1150)
				this._1150.style.color = "gray";
			if (!this._24._1159)
				this._24._1159 = this._24.onclick;
			this._24.onclick = null;
			if (!this._23._1159)
				this._23._1159 = this._23.onclick;
			this._23.onclick=null;
		}
	}
	_1144._1160=function (value)
	{
		this._1152=value;
		this._1153(false);
	}
	_1144._24.onclick=function()
	{
		if(!this._1144._1158)
			return;
		this._1144._1154(this);
	}
	_1144._23.onclick=function()
	{
		if(!this._1144._1158)
			return;
		this._1144._1151(this);
	}
	_1144._1156(true);
	_1144._1153(false);
}

