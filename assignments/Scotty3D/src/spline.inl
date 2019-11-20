// Given a time between 0 and 1, evaluates a cubic polynomial with
// the given endpoint and tangent values at the beginning (0) and
// end (1) of the interval.  Optionally, one can request a derivative
// of the spline (0=no derivative, 1=first derivative, 2=2nd derivative).
template <class T>
inline T Spline<T>::cubicSplineUnitInterval(
	const T& position0, const T& position1, const T& tangent0,
	const T& tangent1, double normalizedTime, int derivative) {
	// TODO (Animation) Task 1a
	double t1 = normalizedTime;
	double t2 = t1 * normalizedTime;
	double t3 = t2 * normalizedTime;
	double h00, h10, h01, h11;
	switch (derivative)
	{
	case 0:
		h00 = 2 * t3 - 3 * t2 + 1;
		h10 = t3 - 2 * t2 + t1;
		h01 = -2 * t3 + 3 * t2;
		h11 = t3 - t2;
		return h00 * position0 + h10 * tangent0 + h01 * position1 + h11 * tangent1;
	case 1:
		h00 = 6 * t2 - 6 * t1;
		h10 = 3 * t2 - 4 * t1 + 1;
		h01 = -6 * t2 + 6 * t1;
		h11 = 3 * t2 - 2 * t1;
		return h00 * position0 + h10 * tangent0 + h01 * position1 + h11 * tangent1;
	case 2:
		h00 = 12 * t1 - 6;
		h10 = 6 * t1 - 4;
		h01 = -12 * t1 + 6;
		h11 = 6 * t1 - 2;
		return h00 * position0 + h10 * tangent0 + h01 * position1 + h11 * tangent1;
	default:
		return T();
	}
	
	}
	

// Returns a state interpolated between the values directly before and after the
// given time.
template <class T>
inline T Spline<T>::evaluate(double time, int derivative) {
	// TODO (Animation) Task 1b
	
	if (knots.size() < 1)
		return T();
	else if (knots.size()==1)
	{
		if (derivative == 0)
		{
			return knots.begin()->second;
		}
		else
		{
			return T();
		}
	}
	else
	{
		map<double, T>::iterator k2_it = knots.upper_bound(time);
		map<double, T>::iterator k1_it = knots.upper_bound(time);
		k1_it--;
		if (k2_it == knots.begin())
		{
			if (derivative == 0)
			{
				return knots.begin()->second;
			}
			else
			{
				return T();
			}
		}

		if (k2_it == knots.end())
		{
			if (derivative == 0)
			{
				return k1_it->second;
			}
			else
			{
				return T();
			}
		}
		
		T p2 = k2_it->second;
		double t2 = k2_it->first;
		T p1 = k1_it->second;
		double t1 = k1_it->first;
		T p0, p3;
		double t0, t3;

		if (k1_it == knots.begin())
		{	
			p0 = p1 - (p2 - p1);
			t0 = t1 - (t2 - t1);
		}
		else
		{
			map<double, T>::iterator k0_it = --k1_it;
			p0 = k0_it->second;
			t0 = k0_it->first;
		}

		map<double, T>::iterator k3_it = ++k2_it;
		if (k3_it == knots.end())
		{
			p3 = p2 + (p2 - p1);
			t3 = t2 + (t2 - t1);		
		}
		else
		{
			p3 = k3_it->second;
			t3 = k3_it->first;
		}

		T m1 = (p2 - p0) / (t2 - t0);
		T m2 = (p3 - p1) / (t3 - t1);
		double t = (time - t1) / (t2 - t1);
		return cubicSplineUnitInterval(p1, p2, m1 * (t2 - t1), m2 * (t2 - t1), t, derivative) / pow(t2 - t1, derivative);

	}
}

// Removes the knot closest to the given time,
//    within the given tolerance..
// returns true iff a knot was removed.
template <class T>
inline bool Spline<T>::removeKnot(double time, double tolerance) {
	// Empty maps have no knots.
	if (knots.size() < 1) {
		return false;
	}

	// Look up the first element > or = to time.
	typename std::map<double, T>::iterator t2_iter = knots.lower_bound(time);
	typename std::map<double, T>::iterator t1_iter;
	t1_iter = t2_iter;
	t1_iter--;

	if (t2_iter == knots.end()) {
		t2_iter = t1_iter;
	}

	// Handle tolerance bounds,
	// because we are working with floating point numbers.
	double t1 = (*t1_iter).first;
	double t2 = (*t2_iter).first;

	double d1 = fabs(t1 - time);
	double d2 = fabs(t2 - time);

	if (d1 < tolerance && d1 < d2) {
		knots.erase(t1_iter);
		return true;
	}

	if (d2 < tolerance && d2 < d1) {
		knots.erase(t2_iter);
		return t2;
	}

	return false;
}

// Sets the value of the spline at a given time (i.e., knot),
// creating a new knot at this time if necessary.
template <class T>
inline void Spline<T>::setValue(double time, T value) {
	knots[time] = value;
}

template <class T>
inline T Spline<T>::operator()(double time) {
	return evaluate(time);
}
