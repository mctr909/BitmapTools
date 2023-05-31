class vec {
	/**
	 * @param {number} x
	 * @param {number} y
	 * @param {number} z
	 */
	constructor(x=0, y=0, z=0) {
		this.X = x;
		this.Y = y;
		this.Z = z;
	}

	get abs() {
		return Math.sqrt(this.X*this.X + this.Y*this.Y + this.Z*this.Z)
	}

	/**
	 * @param {vec} dest 
	 */
	copy(dest) {
		dest.X = this.X;
		dest.Y = this.Y;
		dest.Z = this.Z;
	}

	distance(v) {
		let sx = v.X - this.X;
		let sy = v.Y - this.Y;
		let sz = v.Z - this.Z;
		return Math.sqrt(sx*sx + sy*sy + sz*sz);
	}

	/**
	 * @param {vec} returnValue
	 */
	normalize(returnValue) {
		let inv = Math.sqrt(this.X*this.X + this.Y*this.Y + this.Z*this.Z);
		inv = 1 / inv;
		returnValue.X = inv * this.X;
		returnValue.Y = inv * this.Y;
		returnValue.Z = inv * this.Z;
	}

	/**
	 * @param {number} scale
	 * @param {vec} returnValue
	 */
	normalizeScale(scale, returnValue) {
		let r = Math.sqrt(this.X*this.X + this.Y*this.Y + this.Z*this.Z);
		scale /= r;
		returnValue.X = scale * this.X;
		returnValue.Y = scale * this.Y;
		returnValue.Z = scale * this.Z;
	}

	/**
	 * @param {vec} offset
	 * @param {vec} returnValue
	 */
	normalizeOfs(offset, returnValue) {
		let sx = this.X - offset.X;
		let sy = this.Y - offset.Y;
		let sz = this.Z - offset.Z;
		let inv = Math.sqrt(sx*sx + sy*sy + sz*sz);
		inv = 1 / inv;
		returnValue.X = inv * sx + offset.X;
		returnValue.Y = inv * sy + offset.Y;
		returnValue.Z = inv * sz + offset.Z;
	}

	/**
	 * @param {number} scale
	 * @param {vec} offset
	 * @param {vec} returnValue
	 */
	normalizeScaleOfs(scale, offset, returnValue) {
		let sx = this.X - offset.X;
		let sy = this.Y - offset.Y;
		let sz = this.Z - offset.Z;
		let r = Math.sqrt(sx*sx + sy*sy + sz*sz);
		scale /= r;
		returnValue.X = scale * sx + offset.X;
		returnValue.Y = scale * sy + offset.Y;
		returnValue.Z = scale * sz + offset.Z;
	}

	/**
	 * @param {number} scale
	 * @param {vec} returnValue
	 */
	scale(scale, returnValue) {
		returnValue.X = scale * this.X;
		returnValue.Y = scale * this.Y;
		returnValue.Z = scale * this.Z;
	}

	/**
	 * @param {number} scale
	 * @param {vec} offset
	 * @param {vec} returnValue
	 */
	scaleOfs(scale, offset, returnValue) {
		var sx = this.X - offset.X;
		var sy = this.Y - offset.Y;
		var sz = this.Z - offset.Z;
		returnValue.X = scale * sx + offset.X;
		returnValue.Y = scale * sy + offset.Y;
		returnValue.Z = scale * sz + offset.Z;
	}

	/**
	 * @param {vec} v
	 * @return {number}
	 */
	dot(v) {
		return (this.X*v.X + this.Y*v.Y + this.Z*v.Z);
	}

	/**
	 * @param {vec} v
	 * @param {vec} returnValue
	 */
	cross(v, returnValue) {
		returnValue.X = this.Y * v.Z - this.Z * v.Y;
		returnValue.Y = this.Z * v.X - this.X * v.Z;
		returnValue.Z = this.X * v.Y - this.Y * v.X;
	}

	/**
	 * @param {vec} v
	 * @param {vec} returnValue
	 */
	add(v, returnValue) {
		returnValue.X = this.X + v.X;
		returnValue.Y = this.Y + v.Y;
		returnValue.Z = this.Z + v.Z;
	}

	/**
	 * @param {vec} v
	 * @param {vec} returnValue
	 */
	sub(v, returnValue) {
		returnValue.X = this.X - v.X;
		returnValue.Y = this.Y - v.Y;
		returnValue.Z = this.Z - v.Z;
	}

	/**
	 * @param {vec} offset
	 * @param {phaser} returnValue
	 */
	toPhaserOfs(offset, returnValue) {
		var sx = this.X - offset.X;
		var sy = this.Y - offset.Y;
		var sz = this.Z - offset.Z;
		var xx_zz = sx*sx + sz*sz;
		returnValue.radius = Math.sqrt(xx_zz + sy*sy);
		returnValue.azimuth = Math.atan2(sz, sx);
		returnValue.elevation = Math.atan2(sy, Math.sqrt(xx_zz));
	}
}

class phaser {
	/**
	 * @param {number} radius
	 * @param {number} azimuth
	 * @param {number} elevation
	 */
	constructor(radius=0, azimuth=0, elevation=0) {
		this.radius = radius;
		this.azimuth = azimuth;
		this.elevation = elevation;
	}
	toVec(returnValue) {
		returnValue.X = this.radius * Math.cos(this.azimuth) * Math.cos(this.elevation);
		returnValue.Y = this.radius * Math.sin(this.elevation);
		returnValue.Z = this.radius * Math.sin(this.azimuth) * Math.cos(this.elevation);
	}
}

/**
 * @param {vec} p
 * @param {vec} a
 * @param {vec} b
 * @param {vec} returnValue
 * @param {number} beginLimit
 * @param {number} endLimit
 * @return {number}
 */
function nearPointOnLine(p, a, b, returnValue, beginLimit=true, endLimit=true) {
	var abx = b.X - a.X;
	var aby = b.Y - a.Y;
	var abz = b.Z - a.Z;
	var apx = p.X - a.X;
	var apy = p.Y - a.Y;
	var apz = p.Z - a.Z;
	var r = (apx*abx + apy*aby + apz*abz) / (abx*abx + aby*aby + abz*abz);
	if (beginLimit && r <= 0) {
		returnValue.X = a.X;
		returnValue.Y = a.Y;
		returnValue.Z = a.Z;
		return  r;
	}
	if (endLimit && 1 <= r) {
		returnValue.X = b.X;
		returnValue.Y = b.Y;
		returnValue.Z = b.Z;
		return r;
	}
	returnValue.X = abx*r + a.X;
	returnValue.Y = aby*r + a.Y;
	returnValue.Z = abz*r + a.Z;
	return r;
}

/**
 * @param {vec} o
 * @param {vec} a
 * @param {vec} b
 * @param {vec} returnValue
 */
function divLine(o, a, b, returnValue) {
	var oax = a.X - o.X;
	var oay = a.Y - o.Y;
	var oaz = a.Z - o.Z;
	var obx = b.X - o.X;
	var oby = b.Y - o.Y;
	var obz = b.Z - o.Z;
	var oar = Math.sqrt(oax*oax + oay*oay + oaz*oaz);
	var obr = Math.sqrt(obx*obx + oby*oby + obz*obz);
	returnValue.X = (oax / oar + obx / obr) + o.X;
	returnValue.Y = (oay / oar + oby / obr) + o.Y;
	returnValue.Z = (oaz / oar + obz / obr) + o.Z;
}

/**
 * @param {vec} o
 * @param {vec} a
 * @param {vec} b
 * @param {vec} returnValue
 */
function crossedDivLine(o, a, b, returnValue) {
	var oax = a.X - o.X;
	var oay = a.Y - o.Y;
	var oaz = a.Z - o.Z;
	var obx = b.X - o.X;
	var oby = b.Y - o.Y;
	var obz = b.Z - o.Z;
	var oar = Math.sqrt(oax*oax + oay*oay + oaz*oaz);
	var obr = Math.sqrt(obx*obx + oby*oby + obz*obz);
	returnValue.X = (obr * oax + oar * obx) / (oar + obr) + o.X;
	returnValue.Y = (obr * oay + oar * oby) / (oar + obr) + o.Y;
	returnValue.Z = (obr * oaz + oar * obz) / (oar + obr) + o.Z;
}
/**
 * @param {number} v
 * @returns {number}
 */
function int(v) {
	return parseInt(v);
}
/**
 * @param {number} v
 * @param {number} unit
 * @returns {number}
 */
function round(v, unit=0.01) {
	return parseInt(v / unit) * unit;
}
/**
 * @param {vec} a
 * @param {vec} b
 * @returns {number}
 */
function distance(a, b) {
	return a.distance(b);
}