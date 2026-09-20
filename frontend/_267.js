// [_267.js] 进度条组件 _1406: 三段图片拼接(_269.png 左帽/_268.png 中段/_270.png 右帽), _1160(value,max) 更新
function _1406(_303,width,height)
{
	if(_303.style.position!="absolute"&&_303.style.position!="relative")
		_303.style.position="relative";
	{
		let _1328;
		_1328=_303.getBoundingClientRect();
		if(width)
		{
			_303.style.display="inline-block";
			_303.style.width=width+"px";
			_303.width=width;
		}
		else
			_303.width=_1328.width;
		if(height)
		{
			_303.style.display="inline-block";
			_303.style.height=height+"px";
			_303.height=height;
		}
		else
			_303.height=_1328.height;
		_303.value=0;
		_303.max=100;
	}

	let background;
	background=document.createElement("div");
	background.style.fontSize="medium";
	background.style.position="absolute";
	background.style.left="0px";
	background.style.top="0px";
	background.style.width=_303.width+"px";
	background.style.height=_303.height+"px";
	background.style.borderStyle = "inset";
	if (height<15)
		background.style.borderWidth = "1px";
	else
		background.style.borderWidth = "2px";
	_303.appendChild(background);
	_303.background=background;

	let left;
	left=document.createElement("img");
	left.src="_269.png";
	left.style.position="absolute";
	left.style.left="3px";
	left.style.top="4px";
	left.style.width="10px";
	left.style.height=(_303.height-3)+"px";
	_303.appendChild(left);
	_303.left=left;

	let _1902;
	_1902=document.createElement("img");
	_1902.src="_268.png";
	_1902.style.position="absolute";
	_1902.style.left="13px";
	_1902.style.top=left.style.top
	_1902.style.width="10px";
	_1902.style.height=left.style.height;
	_303.appendChild(_1902);
	_303._1902=_1902;

	let right;
	right=document.createElement("img");
	right.src="_270.png";
	right.style.position="absolute";
	right.style.left="23px";
	right.style.top=left.style.top
	right.style.width=left.style.width;
	right.style.height=left.style.height;
	_303.appendChild(right);
	_303.right=right;

	_303._1160=function(value,max)
	{
		this.value=value;
		this.max=max;
		let background,left,_1902,right;
		background=this.background;
		left=this.left;
		_1902=this._1902;
		right=this.right;

		let margin;
		margin=parseInt(left.style.left);
		let _1903;
		_1903=_303.width;
		_1903-=margin;
		let _1904=10;

		let length;
		if(max==0)
			length=1;
		else
			length=parseInt(_1903*value/max);
		if(length<_1904*2)
		{
			_1904=length/2;
			left.style.width=(_1904)+"px";
			_1902.style.left=(margin+_1904)+"px";
			_1902.style.width="1px";
			right.style.width=(_1904)+"px";
			right.style.left=(margin+_1904+1)+"px";
		}
		else
		{
			left.style.width=(_1904)+"px";
			_1902.style.left=(margin+_1904)+"px";
			_1902.style.width=(length-_1904*2)+"px";
			right.style.width=(_1904)+"px";
			right.style.left=(margin+length-_1904)+"px";
		}
	};

	_303._1346 = function (_1157)
	{
		if (_1157)
		{
			left.style.webkitFilter = "";
			_1902.style.webkitFilter = "";
			right.style.webkitFilter = "";
		}
		else
		{
			left.style.webkitFilter = "grayscale(100%)";
			_1902.style.webkitFilter = "grayscale(100%)";
			right.style.webkitFilter = "grayscale(100%)";
		}
	}

	_303._1905 = function (width, height)
	{
		_303.style.width = width + "px";
		_303.width = width;
		_303.style.height = height + "px";
		_303.height = height;

		background.style.width = _303.width + "px";
		background.style.height = _303.height + "px";

		if (height < 15)
			background.style.borderWidth = "1px";
		else
			background.style.borderWidth = "2px";

		left.style.height = (_303.height - 3) + "px";
		_1902.style.height = left.style.height;
		right.style.height = left.style.height;

		_303._1160(_303.value, _303.max);

	}
}
