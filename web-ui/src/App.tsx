import { useSliderParam, useToggleParam, useChoiceParam } from './hooks/useJuceParam';
import { GateVisualizer } from './components/GateVisualizer';
import './index.css';

const patternNames = ['All', 'Alternate', 'Quarter', 'Half', 'Trance', 'Sidechain', 'Syncopated', 'Stutter'];
const rateNames = ['1/1', '1/2', '1/4', '1/8', '1/16', '1/32'];

function App() {
  // Pattern Parameters
  const pattern = useChoiceParam('pattern', 8, 4);
  const steps = useSliderParam('steps', 16);
  const rate = useChoiceParam('rate', 6, 3);

  // Envelope Parameters
  const attack = useSliderParam('attack', 5);
  const hold = useSliderParam('hold', 50);
  const release = useSliderParam('release', 50);
  const curve = useSliderParam('curve', 0);

  // Modulation Parameters
  const swing = useSliderParam('swing', 0);
  const humanize = useSliderParam('humanize', 0);
  const velocity = useSliderParam('velocity', 0);

  // Mix Parameters
  const depth = useSliderParam('depth', 100);
  const mix = useSliderParam('mix', 100);
  const output = useSliderParam('output', 0);
  const bypass = useToggleParam('bypass', false);

  return (
    <div className={`app ${bypass.value ? 'bypassed' : ''}`}>
      <header className="header">
        <h1 className="title">GATE</h1>
        <span className="subtitle">Rhythmic Trance Gate</span>
        <button
          className={`bypass-btn ${bypass.value ? 'active' : ''}`}
          onClick={bypass.toggle}
        >
          {bypass.value ? 'BYPASSED' : 'ACTIVE'}
        </button>
      </header>

      <div className="visualizer-section">
        <GateVisualizer numSteps={Math.round(steps.value)} pattern={pattern.value} />
      </div>

      {/* Pattern Section */}
      <div className="pattern-section">
        <div className="pattern-selector">
          {patternNames.map((name, i) => (
            <button
              key={name}
              className={`pattern-btn ${pattern.value === i ? 'active' : ''}`}
              onClick={() => pattern.setChoice(i)}
            >
              {name}
            </button>
          ))}
        </div>
        <div className="rate-controls">
          <div className="steps-control">
            <label>Steps</label>
            <div className="steps-value">{Math.round(steps.value)}</div>
            <input
              type="range"
              min={4}
              max={16}
              step={1}
              value={steps.value}
              onChange={(e) => steps.setValue(parseInt(e.target.value))}
              onMouseDown={steps.dragStart}
              onMouseUp={steps.dragEnd}
            />
          </div>
          <div className="rate-selector">
            <label>Rate</label>
            <div className="rate-buttons">
              {rateNames.map((name, i) => (
                <button
                  key={name}
                  className={`rate-btn ${rate.value === i ? 'active' : ''}`}
                  onClick={() => rate.setChoice(i)}
                >
                  {name}
                </button>
              ))}
            </div>
          </div>
        </div>
      </div>

      <div className="controls-grid">
        {/* Envelope Section */}
        <div className="section">
          <h2 className="section-title">Envelope</h2>
          <div className="controls-row">
            <Knob
              label="Attack"
              value={attack.value}
              onChange={attack.setValue}
              onDragStart={attack.dragStart}
              onDragEnd={attack.dragEnd}
              min={0.1}
              max={100}
              unit="ms"
            />
            <Knob
              label="Hold"
              value={hold.value}
              onChange={hold.setValue}
              onDragStart={hold.dragStart}
              onDragEnd={hold.dragEnd}
              min={0}
              max={100}
              unit="%"
            />
            <Knob
              label="Release"
              value={release.value}
              onChange={release.setValue}
              onDragStart={release.dragStart}
              onDragEnd={release.dragEnd}
              min={0.1}
              max={500}
              unit="ms"
            />
            <Knob
              label="Curve"
              value={curve.value}
              onChange={curve.setValue}
              onDragStart={curve.dragStart}
              onDragEnd={curve.dragEnd}
              min={-100}
              max={100}
              bipolar
            />
          </div>
        </div>

        {/* Modulation Section */}
        <div className="section">
          <h2 className="section-title">Feel</h2>
          <div className="controls-row">
            <Knob
              label="Swing"
              value={swing.value}
              onChange={swing.setValue}
              onDragStart={swing.dragStart}
              onDragEnd={swing.dragEnd}
              min={0}
              max={100}
              unit="%"
            />
            <Knob
              label="Humanize"
              value={humanize.value}
              onChange={humanize.setValue}
              onDragStart={humanize.dragStart}
              onDragEnd={humanize.dragEnd}
              min={0}
              max={100}
              unit="%"
            />
            <Knob
              label="Velocity"
              value={velocity.value}
              onChange={velocity.setValue}
              onDragStart={velocity.dragStart}
              onDragEnd={velocity.dragEnd}
              min={0}
              max={100}
              unit="%"
            />
          </div>
        </div>

        {/* Output Section */}
        <div className="section">
          <h2 className="section-title">Output</h2>
          <div className="controls-row">
            <Knob
              label="Depth"
              value={depth.value}
              onChange={depth.setValue}
              onDragStart={depth.dragStart}
              onDragEnd={depth.dragEnd}
              min={0}
              max={100}
              unit="%"
            />
            <Knob
              label="Mix"
              value={mix.value}
              onChange={mix.setValue}
              onDragStart={mix.dragStart}
              onDragEnd={mix.dragEnd}
              min={0}
              max={100}
              unit="%"
              highlight
            />
            <Knob
              label="Output"
              value={output.value}
              onChange={output.setValue}
              onDragStart={output.dragStart}
              onDragEnd={output.dragEnd}
              min={-24}
              max={12}
              unit="dB"
              bipolar
            />
          </div>
        </div>
      </div>
    </div>
  );
}

// Knob Component
interface KnobProps {
  label: string;
  value: number;
  onChange: (value: number) => void;
  onDragStart?: () => void;
  onDragEnd?: () => void;
  min: number;
  max: number;
  unit?: string;
  decimals?: number;
  highlight?: boolean;
  bipolar?: boolean;
}

function Knob({ label, value, onChange, onDragStart, onDragEnd, min, max, unit = '', decimals = 1, highlight, bipolar }: KnobProps) {
  const normalized = (value - min) / (max - min);
  const angle = -135 + normalized * 270;

  const handleMouseDown = (e: React.MouseEvent) => {
    e.preventDefault();
    onDragStart?.();

    const startY = e.clientY;
    const startValue = value;
    const range = max - min;

    const handleMouseMove = (e: MouseEvent) => {
      const delta = (startY - e.clientY) / 150;
      const newValue = Math.max(min, Math.min(max, startValue + delta * range));
      onChange(newValue);
    };

    const handleMouseUp = () => {
      onDragEnd?.();
      window.removeEventListener('mousemove', handleMouseMove);
      window.removeEventListener('mouseup', handleMouseUp);
    };

    window.addEventListener('mousemove', handleMouseMove);
    window.addEventListener('mouseup', handleMouseUp);
  };

  return (
    <div className={`knob-container ${highlight ? 'highlight' : ''}`}>
      <div className="knob" onMouseDown={handleMouseDown}>
        <svg viewBox="0 0 100 100" className="knob-svg">
          <circle
            cx="50"
            cy="50"
            r="40"
            fill="none"
            stroke="#333"
            strokeWidth="4"
            strokeDasharray="188.5 251.3"
            strokeDashoffset="-31.4"
            strokeLinecap="round"
          />
          {bipolar ? (
            <circle
              cx="50"
              cy="50"
              r="40"
              fill="none"
              stroke="var(--accent-color)"
              strokeWidth="4"
              strokeDasharray={`${Math.abs(normalized - 0.5) * 188.5} 251.3`}
              strokeDashoffset={normalized >= 0.5 ? "-125.6" : `${-125.6 - (0.5 - normalized) * 188.5}`}
              strokeLinecap="round"
              className="knob-track-active"
            />
          ) : (
            <circle
              cx="50"
              cy="50"
              r="40"
              fill="none"
              stroke="var(--accent-color)"
              strokeWidth="4"
              strokeDasharray={`${normalized * 188.5} 251.3`}
              strokeDashoffset="-31.4"
              strokeLinecap="round"
              className="knob-track-active"
            />
          )}
          <g transform={`rotate(${angle} 50 50)`}>
            <line
              x1="50"
              y1="20"
              x2="50"
              y2="30"
              stroke="var(--accent-color)"
              strokeWidth="3"
              strokeLinecap="round"
            />
          </g>
        </svg>
      </div>
      <div className="knob-value">
        {decimals === 0 ? Math.round(value) : value.toFixed(decimals)}{unit}
      </div>
      <div className="knob-label">{label}</div>
    </div>
  );
}

export default App;
