// [_262.js] 基础工具库: class _878 (Misc) —— UI 使能/禁用、base64 编解码(_1824/_1838)、
//   WAV 封装(_1874)、hexdump(_1117)、线性插值等; class _1896 可播种随机数。
//   完整符号表见 SYMBOLS.md
class _878
{
	static _879(_1144,_1157)
	{
		if(!_1144.getAttribute("enable"))
		{
			_1144.setAttribute("enable","true");
			_1144._1159 = _1144.onclick;
			_1144._1732 = _1144.onmousedown;
			_1144._1733 = _1144.style.cursor;
		}
		var _1734,text;
		_1734 = _1144.getElementsByTagName("img")[0];
		if (!_1734)
			_1734 = _1144;
		text=_1144.getElementsByTagName("span")[0];
		if(_1157)
		{
			_1144.setAttribute("enable", "true");
			if(_1734)
				_1734.style.webkitFilter = "";
			if(text)
				text.style.color="";
			_1144.style.cursor = _1144._1733;
			_1144.onclick = _1144._1159;
			_1144.onmousedown = _1144._1732;
		}
		else
		{
			_1144.setAttribute("enable", "false");
			if(_1734)
				_1734.style.webkitFilter = "grayscale(100%)";
			if (text)
				text.style.color="gray";
			_1144.style.cursor="";
			_1144.onclick = null;
			_1144.onmousedown = null;
		}
	}

	static _1735=new Array();
	static _1736(_1737)
	{
		var _1738=document.createElement("script");
		_1738.onload=function()
		{
			let _1735;
			_1735=_878._1735;
			if(_1735[0]._1145)
				_1735[0]._1145();
			_1735.shift();
			if(_1735.length)
				_878._1736(_1735[0]);
		}
		_1738.setAttribute('type','text/javascript');
		_1738.setAttribute('src',_1737._1739);
		document.getElementsByTagName('head')[0].appendChild(_1738);
	}
	static _978(_1739,_1145)
	{
		let _1740=new Object();
		_1740._1739=_1739;
		_1740._1145=_1145;
		this._1735.push(_1740);
		if(this._1735.length==1)
			this._1736(_1740);
	}
	static _1741(_917=-1)
	{
		function _1742()
		{
			document.removeEventListener("mouseup",_1742,true);
			document.onselectstart = null;
			document.body.style.cursor = "";
			if (_917 != -1)
			{
				if (_1743 != _1744._1745.bind(_882[_917])(A("_9730")))
					_919._920(_917);
			}
			_878._1746();
		};
		document.addEventListener("mouseup", _1742, true);
		document.onselectstart=function()
		{
			return false;
		}
		let cursor;
		cursor = event.target.style.cursor;
		document.body.style.cursor = cursor;
		let _1743;
		if (_917 != -1)
			_1743 = _1744._1745.bind(_882[_917])(A("_9730"));
		_878._1747();
	}
	static _1746()
	{
		if (this._1748())
			parent._1749();
		else
			_928._1746();
	}
	static _1747()
	{
		if (this._1748())
			parent._1750();
		else
			_928._1747();
	}



	static _1751(_1232)
	{
		
		
		return /^[a-zA-Z\'\.\,\s\-]+$/.test(_1232);
	}
	static _1752 (_1753)
	{
		if (_1753 <= 0.5)
			return _1753 * 2;
		return 1 + (10 - 1) * (_1753 - 0.5) / (1 - 0.5);
	};

	static _1754(_1755)
	{
		if(_1755<=1)
			return _1755/2;
		return 0.5+0.5*(_1755-1)/(10-1);
	}
	static _1752(_1753)
	{
		if(_1753<=0.5)
			return _1753*2;
		return 1+(10-1)*(_1753-0.5)/(1-0.5);
	}
	static _1756(_1757)
	{
		if(_1757<=50)
			return _1757/50;
		return 1+(10-1)*(_1757-50)/50;
	}
	static _1758(_1755)
	{
		if(_1755<=1)
			return _1755*50;
		
		return 50+50*(_1755-1)/(10-1);
	}
	static _1759(_1757)
	{
		if(_1757<=50)
			return 0.1+(1-0.1)*_1757/50;
		return 1+(10-1)*(_1757-50)/50;
	}
	static _1760(_1761)
	{
		if(_1761<=1)
			return 50*(_1761-0.1)/(1-0.1);
		return 50+50*(_1761-1)/(10-1);
	}
	static _1066(_835, y1, _996, x1, x)
	{
		if (x1 == _996)
			return (_835 + y1) / 2;
		return _835 + (y1 - _835) * (x - _996) / (x1 - _996);
	}

	static _1762(x, y, _1763)
	{
		let _1764;
		let _1765;
		_1765 = x * x + y * y;
		_1765 = Math.sqrt(_1765);
		if (_1765 == 0)
			_1764 = 0;
		else
		{
			_1764 = Math._1766(x / _1765);
			if (y < 0)
				_1764 = Math.PI * 2 - _1764;
		}
		if (_1763)
			_1763(_1765);
		return _1764;
	}



	static _1770(result,_1111)
	{
		if (_1111 == null)
			return null;
		if (Array.isArray(result) != Array.isArray(_1111))
			console.assert(0);
		if (Array.isArray(result))
			result.length = 0;
		for (let key in result)
			if (typeof result[key] != "function")
				delete result[key];
		for (let key in _1111)
		{
			if (_1111.hasOwnProperty(key))
			{
				let _1768;
				_1768 = _1111[key];
				if (typeof _1768 != 'function')
				{
					if (typeof _1768 == 'object')
						result[key] = _878._1769(_1768);
					else
						result[key] = _1768;
				}
			}
		}
    }
	static _1769(_1111)
	{
		if (_1111 == null)
			return null;
		let _1771;
		_1771 = Array.isArray(_1111);
		let result;
		if (_1771)
			result = new Array();
		else
			result = new Object();
		this._1770(result, _1111);
		return result;
	}

	static _1772(_1111)
	{
		if(_1111==null)
			return null;
		let _1771;
		_1771 = Array.isArray(_1111);
		let result;
		if (_1771)
			result = new Array();
		else
			result = new Object();
		for(let key in _1111)
		{
			if(_1111.hasOwnProperty(key))
			{
				let _1768;
				_1768 = _1111[key];
				if (typeof _1768 == 'object')
					result[key] = _878._1772(_1768); 
				else if (typeof _1768 == 'function')
					result[key] = _878._1773(_1768,result);
				else
					result[key] = _1768;
			}
		}

		if (!_1771)
		{
			let _1774;
			_1774 = _1111.constructor;
			if (_1774)
			{
				var propertyNames = Object.getOwnPropertyNames(_1774.prototype);
				for (let _1059 = 0; _1059 < propertyNames.length; _1059++)
				{
					let _1664 = propertyNames[_1059];
					if (_1664 == "constructor" || _1664 == "__defineGetter__" || _1664 == "__defineSetter__" || _1664 == "hasOwnProperty" || _1664 == "__lookupGetter__" || _1664 == "__lookupSetter__" || _1664 == "isPrototypeOf" || _1664 == "propertyIsEnumerable" || _1664 == "toString" || _1664 == "valueOf" || _1664 == "__proto__" || _1664 == "toLocaleString")
						continue;
					let _1775 = _1774.prototype[_1664];
					if (typeof _1775 == "function")
						result[_1664] = _878._1773(_1775, result);
					else
						result[_1664] = _1775;
				}
			}
		}
		return result;
	}
	static _1776(_1702)
	{
		var _865;
		_865 = _1702;
		_865 = _865.replace(/\\/g, "\\\\");
		_865 = _865.replace(new RegExp("\"", "gm"), "\\\"");
		_865 = _865.replace(new RegExp("\'", "gm"), "\\\'");
		return _865;
	}
	static _1777(_1702)
	{
		var _865;
		_865 = _1702;
		_865 = _865.replace(new RegExp("&", "gm"), "&amp;");
		_865 = _865.replace(new RegExp("<", "gm"), "&lt;");
		_865 = _865.replace(new RegExp(">", "gm"), "&gt;");
		return _865;
	}
	static _1748()
	{
		if (self.frameElement && self.frameElement.tagName == "IFRAME")
			return true;
		return false;
	}
	static _1773(_1778,_1162)
	{
		if (_1778._1779)
			_1778 = _1778._1779;
		let _865;
		_865 = _1778.bind(_1162);
		_865._1779 = _1778;
		return _865;
	}
	static _1780(_1781,_1782)
	{
		if (_1781 == -1)
			return -1;
		if (_1781 == _1782)
			return -1;
		if (_1782 < _1781)
			return _1781 - 1;
		return _1781;
    }
	static _1783(_1781, _1784)
	{
		if (_1781 == -1)
			return -1;
		if (_1781 >= _1784)
			_1781++;
		return _1781;
	}
	
	
	
	
	
	static _1785(_1786)
	{
		return _1786.toString().match(/\/\*\s([\s\S]*)\s\*\//)[1];
	}
	static _1787(_1702, _1788)
	{
		return _1702.substr(_1702.length - _1788);
	}
	static _1653(_1702,_1788)
	{
		return _1702.substr(0,_1788);
	}
	static _1683(_1702,_1789)
	{
		if (_1702.length >= _1789.length)
			if (_878._1653(_1702,_1789.length) == _1789)
				return true;
		return false;
	}
	static _1652(_1702, _1790)
	{
		if (_1702.length >= _1790.length)
			if (_878._1787(_1702,_1790.length) == _1790)
				return true;
		return false;
	}
	static _1791(_1702, _1792, substring)
	{
		let _865;
		let _1793, _1794;
		if (_1792 < 0)
			_1792 = 0;
		if (_1792 < _1702.length)
			_1793 = _878._1653(_1702, _1792);
		else
			_1793 = _1702;
		if (_1792 + substring.length < _1702.length)
			_1794 = _1702.substr(_1792 + substring.length);
		else
			_1794 = "";
		_865 = _1793 + substring +_1794;
		return _865;
	}
	static _1795()
	{
		switch (navigator.language.toLowerCase())
		{
			case "zh-cn":
			case "zh-sg":
			case "zh-tw":
				return "chinese";
			case "ja":
			case "ja-jp":
				return "japanese";
			case "zh-hk":
			case "yue":
			case "zh-yue":
			case "zh-yue-hk":
			case "yue-hk":
				return "cantonese";
			
			default:
				return "english";
		}
	}
	static _1796(_1797,_1479,_1798)
	{
		if (_1797 < 0)
			_1797 = 0;
		if (_1797 > 1)
			_1797 = 1;
		let _1799;
		function _1800()
		{
			
			
			
			
			
			if (_1797 < 4 / 6.)
				_1797 = this._1066(0., 4 / 6., 4 / 6., 0., _1797);
			else
				_1797 = this._1066(4 / 6., 1., 1., 4 / 6., _1797);
			_1799 = Math.floor(_1797 * (255 * 6 - 1));
		}
		if ((!_1798) && _1479)
			_1799 = Math.floor(_1797 * (255 * 6 - 1));
		else if ((!_1798) && (!_1479))
			_1799 = Math.floor(_1797 * 255 * 5);
		else if (_1798 && _1479)
			_1800.bind(this)();
		else
		{
			_1797 *= (5 / 6.);
			_1800.bind(this)();
		}
			
		let _1801, _1802, _1803;
		let value;
		if (_1799 < 255)
		{
			value=_1799;
			_1801 = 255;
			_1802 =value ;
			_1803 = 0;
		}
		else if (_1799 < 255 * 2)
		{
			value=_1799-255;
			_1801 = 255 -value ;
			_1802 = 255;
			_1803 = 0;
		}
		else if (_1799 < 255 * 3)
		{
			value=_1799-255 * 2;
			_1801 = 0;
			_1802 = 255;
			_1803 =value ;
		}
		else if (_1799 < 255 * 4)
		{
			value=_1799-255 * 3;
			_1801 = 0;
			_1802 = 255 -value ;
			_1803 = 255;
		}
		else if (_1799 < 255 * 5)
		{
			value=_1799-255 * 4;
			_1801 =value ;
			_1802 = 0;
			_1803 = 255;
		}
		else if (_1799 < 255 * 6)
		{
			value=_1799-255 * 5;
			_1801 = 255;
			_1802 = 0;
			_1803 = 255 -value ;
		}
		return [_1801, _1802, _1803];
	}
	static _1804(_1805,_1806)
	{
		let _1807 = _1805[0].length;
		let _1808, _1809, _1810, _1811;
		for (let _1812 = 1; _1812 < _1807; _1812++)
		{
			let _1813;
			_1813 = _1812 - 1;
			let _1814, _1815;
			_1814 = _1813 / (_1807 - 1);
			_1815 = _1812 / (_1807 - 1);
			if (!(_1814 <= _1806 && _1806 <= _1815))
				continue;
			let _1816, _1817, _1818, _1819;
			let _1820, _1821, _1822, _1823;
			_1816 = _1805[0][_1813];
			_1817 = _1805[1][_1813];
			_1818 = _1805[2][_1813];
			_1819 = _1805[3][_1813];
			_1820 = _1805[0][_1812];
			_1821 = _1805[1][_1812];
			_1822 = _1805[2][_1812];
			_1823 = _1805[3][_1812];
			_1808 = _878._1066(_1816, _1820, _1814, _1815, _1806);
			_1809 = _878._1066(_1817, _1821, _1814, _1815, _1806);
			_1810 = _878._1066(_1818, _1822, _1814, _1815, _1806);
			_1811 = _878._1066(_1819, _1823, _1814, _1815, _1806);
			break;
		}
		return [_1808, _1809, _1810, _1811];
	}


	static _1824(_1825, _1826)
	{
		function _1827(_1828)
		{
			return _1828 > 64 && _1828 < 91
				? _1828 - 65
				: _1828 > 96 && _1828 < 123
					? _1828 - 71
					: _1828 > 47 && _1828 < 58
						? _1828 + 4
						: _1828 === 43
							? 62
							: _1828 === 47
								? 63
								: 0;
		}

		const _1829 = _1825.replace(/[^A-Za-z0-9+/]/g, ""); 
		const _1830 = _1829.length;
		const _1831 = _1826
			? Math.ceil(((_1830 * 3 + 1) >> 2) / _1826) * _1826
			: (_1830 * 3 + 1) >> 2;
		const _1832 = new Uint8Array(_1831);

		let _1833;
		let _1834;
		let _1835 = 0;
		let _1836 = 0;
		for (let _1837 = 0; _1837 < _1830; _1837++)
		{
			_1834 = _1837 & 3;
			_1835 |= _1827(_1829.charCodeAt(_1837)) << (6 * (3 - _1834));
			if (_1834 === 3 || _1830 - _1837 === 1)
			{
				_1833 = 0;
				while (_1833 < 3 && _1836 < _1831)
				{
					_1832[_1836] = (_1835 >>> ((16 >>> _1833) & 24)) & 255;
					_1833++;
					_1836++;
				}
				_1835 = 0;
			}
		}

		return _1832;
	}
	static _1703(_1825, _1826)
	{
		return this._1824(_1825, _1826).buffer;
	}


	
	static _1838(_1839)
	{
		function _1840(_1841)
		{
			return _1841 < 26
				? _1841 + 65
				: _1841 < 52
					? _1841 + 71
					: _1841 < 62
						? _1841 - 4
						: _1841 === 62
							? 43
							: _1841 === 63
								? 47
								: 65;
		}

		let _1833 = 2;
		let _1829 = "";

		const _1842 = _1839.length;
		let _1835 = 0;
		for (let _1843 = 0; _1843 < _1842; _1843++)
		{
			_1833 = _1843 % 3;
			
			
			
			

			_1835 |= _1839[_1843] << ((16 >>> _1833) & 24);
			if (_1833 === 2 || _1839.length - _1843 === 1)
			{
				_1829 += String.fromCodePoint(
					_1840((_1835 >>> 18) & 63),
					_1840((_1835 >>> 12) & 63),
					_1840((_1835 >>> 6) & 63),
					_1840(_1835 & 63),
				);
				_1835 = 0;
			}
		}
		return (
			_1829.substring(0, _1829.length - 2 + _1833) +
			(_1833 === 2 ? "" : _1833 === 1 ? "=" : "==")
		);
	}
	static _1693(_1839)
	{
		return this._1838(new Uint8Array(_1839));
	}

	static _1396(_1844)
	{
		let _1328 = _1844.getBoundingClientRect();
		if (event.clientX >= _1328.left && event.clientX <= _1328.right && event.clientY >= _1328.top && event.clientY <= _1328.bottom)
			return true;
		return false;
	}

	static _1845(_1846, _1847)
	{
		let _1848;
		_1848 = _1846 % _1847;
		if (_1848 < 0)
			_1848 += _1847;
		return _1848;
	}

	static _1340(x)
	{
		while (true)
		{
			if (!x)
				return true;
			if (x.style.display == "none")
				return false;
			x = x.parentElement;
		}
	}

	static _1849(_1850, _1160, _1851, _1852, _1853, _1854)
	{
		let _381 = document.createElement('input');
		_381.type = 'text';
		document.body.appendChild(_381);
		_381.style.position = "absolute";
		_381.style.left = _1852 + 'px';
		_381.style.top = _1853 + 'px';
		if (!_1850())
			_381.value = L(_1854);
		else
			_381.value = _1850();
		_381.oninput = function ()
		{
			let value;
			value = _381.value;
			if (value == L(_1854))
				value = "";
			_1160(value);
			if (_1851)
				_1851();
		}
		_381.onfocus = function ()
		{
			if (this.value == L(_1854))
				this.select();
		}
		function _1855()
		{
			event.stopPropagation();
			document.body.removeChild(_381);
			document.removeEventListener('click', _1856, true);
			document.removeEventListener("keyup", _1857, true);
			_381 = null;
		}

		function _1857()
		{
			if (event.key === 'Enter')
				_1855();
		}
		document.addEventListener("keyup", _1857, true);

		_381.focus();
		function _1856()
		{
			if (event.target !== _381)
				_1855()
		}
		document.addEventListener('click', _1856, true);
	}
	static _1858(_1669, _1859)
	{
		let _1860, _1861;
		_1860 = Math.floor(_1859);
		_1861 = _1860 + 1;
		let left, right;
		left = _1669[Math.max(Math.min(_1860, _1669.length - 1), 0)];
		right = _1669[Math.max(Math.min(_1861, _1669.length - 1), 0)];
		let _1862;
		_1862 = _1859 - _1860;
		let _1863;
		_1863 = 1 - _1862;
		return left * _1863 + right * _1862;
	}
	static _1864(_1865, _1866)
	{
		let _1867;
		_1867 = _1865.length;
		let _1868;
		_1868 = Math.round(_1867 * _1202._1869 / _1866);
		let _1870;
		if (Math.abs(_1868 - 1) < 1e-9)
			_1870 = 1;
		else
			_1870 = (_1867 - 1) / (_1868 - 1);
		let _1871;
		_1871 = new Float32Array(_1868);
		for (let _1872 = 0; _1872 < _1868; _1872++)
		{
			let _1873;
			_1873 = _1872 * _1870;
			_1871[_1872] = _878._1858(_1865, _1873);
		}
		return _1871;
	}

	static _1874(_1875, _1876, _1877=1)
	{
		
		let _1878 = 16;
		let _1879 = true
		let _1880;
		_1880 = new DataView(_1875.buffer);
		function _937(data, _1120, _1881)
		{
			for (var _1119 = 0; _1119 < _1881.length; _1119++)
			{
				data.setUint8(_1120 + _1119, _1881.charCodeAt(_1119))
			}
		}
		let _1882 = _1878
		let buffer = new ArrayBuffer(44 + _1880.byteLength)
		let data = new DataView(buffer)
		let _1883 = _1877
		let _1120 = 0
		
		_937(data, _1120, 'RIFF')
		_1120 += 4
		
		data.setUint32(_1120, 36 + _1880.byteLength, true)
		_1120 += 4
		
		_937(data, _1120, 'WAVE')
		_1120 += 4
		
		_937(data, _1120, 'fmt ')
		_1120 += 4
		
		data.setUint32(_1120, 16, true)
		_1120 += 4
		
		data.setUint16(_1120, 1, true)
		_1120 += 2
		
		data.setUint16(_1120, _1883, true)
		_1120 += 2
		
		data.setUint32(_1120, _1876, true)
		_1120 += 4
		
		data.setUint32(_1120, _1883 * _1876 * (_1882 / 8), true)
		_1120 += 4
		
		data.setUint16(_1120, _1883 * (_1882 / 8), true)
		_1120 += 2
		
		data.setUint16(_1120, _1882, true)
		_1120 += 2
		
		_937(data, _1120, 'data')
		_1120 += 4
		
		data.setUint32(_1120, _1880.byteLength, true)
		_1120 += 4

		
		for (let _1119 = 0; _1119 < _1880.byteLength;)
		{
			data.setUint8(_1120, _1880.getUint8(_1119), true)
			_1120++
			_1119++
		}

		return data
	}


	static _1884(_1885, _1886, _1887, _1888)
	{
		return _1885 <= _1888 && _1886 >= _1887;
	}
	static _1889(_1890, _1891, _1892, _1893)
	{
		let start, _1790;
		start = Math.max(_1890, _1892);
		_1790 = Math.min(_1891, _1893);
		if (start <= _1790)
			return [start, _1790];
		return null;
	}
	static _1894(_1702)
	{
		if (_1702 && _1702.charAt(0) === '\uFEFF')
		{
			_1702 = _1702.slice(1);
			if (_1702 && _1702.charAt(0) === '\uFEFF')
				_1702 = _1702.slice(1);
		}
		return _1702;
	}
	static _1117(_939)
	{
		if (!_939)
			return "";
		const _1118 = new Uint8Array(_939);
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
			result += _1120 + '  ' + _1121 + padding + '  ' + _1122 + '\n';
		}
		return result.trim();
	}
}
function _1895(_1144,_1157)
{
	_878._879(_1144,_1157);
}




function CMisc_GetLongString(_1786)
{
	return _1786.toString().match(/\/\*\s([\s\S]*)\s\*\//)[1];
}
function _1749()
{
	_878._1746();
}
function _1750()
{
	_878._1747();
}

class _1896
{
	_1897=5;
	_1898(_1897)
	{
		this._1897=_1897;
	}
	_1008()
	{
		this._1897=(this._1897*9301+49297)%233280;
		return this._1897/233280.0;
	}
}



function _1899(_1900)
{
	return "({get value(){return " + _1900 + ";}, set value(v){" + _1900 + "=v;}})";
}
function _1076(_1901, value)
{
	_1901.value = value;
}

